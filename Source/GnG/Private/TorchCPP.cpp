// Fill out your copyright notice in the Description page of Project Settings.

#include "TorchCPP.h"
#include "Projectile_Base.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

ATorchCPP::ATorchCPP()
{
	FlameTrailEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Flame Trail Effect"));
	// Keep the effect attached and ready, but do not start it until the torch actually impacts something.
	FlameTrailEffect->SetupAttachment(CollisionBox);
	FlameTrailEffect->SetAutoActivate(false);
	FlameTrailEffect->SetRelativeLocation(FVector(40.f, 0.f, 0.f));
	FlameTrailEffect->SetRelativeScale3D(FlameTrailScale);

	// Torches are meant to arc downward more than the base projectile.
	ProjectileMovement->ProjectileGravityScale = 3.5f;
}

void ATorchCPP::ApplyAreaDamage()
{
	if (!GetWorld())
	{
		return;
	}

	// The overlap query collects every actor close enough to the impact point to be inside the torch blast.
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(AOERadius);
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(TorchAOE), false, this);
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner());

	GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		CollisionShape,
		QueryParams
	);

	// Unreal overlap queries can return multiple components from the same actor, so we dedupe here.
	TSet<AActor*> UniqueTargets;
	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* TargetActor = Result.GetActor();
		if (!TargetActor || TargetActor == GetOwner() || UniqueTargets.Contains(TargetActor))
		{
			continue;
		}

		UniqueTargets.Add(TargetActor);

		// Apply the immediate explosion damage first so the impact still feels responsive.
		FDamageEvent DamageEvent(UDamageType::StaticClass());
		TargetActor->TakeDamage(ImpactDamage, DamageEvent, GetInstigatorController(), this);

		if (bApplyBurningDOT)
		{
			// Queue the actor for future timer ticks instead of applying all DOT damage immediately.
			ApplyBurningEffect(TargetActor);
		}
	}

	// Start one repeating timer after impact to keep all burn targets in sync.
	if (bApplyBurningDOT && BurningTargets.Num() > 0)
	{
		// Track the remaining lifetime explicitly so the timer duration stays easy to tune in Blueprint.
		BurnTimeRemaining = DamageDuration;
		GetWorldTimerManager().SetTimer(BurnTimerHandle, this, &ATorchCPP::HandleBurnTick, DamageTickInterval, true);
	}
}

void ATorchCPP::ApplyBurningEffect(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	// A target should only be added once, otherwise the burn would stack accidentally every time the overlap query sees it.
	for (const FTorchBurnTarget& BurnTarget : BurningTargets)
	{
		if (BurnTarget.Target == Target)
		{
			return;
		}
	}

	FTorchBurnTarget NewBurnTarget;
	NewBurnTarget.Target = Target;
	BurningTargets.Add(NewBurnTarget);
}

void ATorchCPP::HandleBurnTick()
{
	// Each timer tick represents one "burn pulse" that damages every actor still in the burn list.
	for (int32 Index = BurningTargets.Num() - 1; Index >= 0; --Index)
	{
		AActor* TargetActor = BurningTargets[Index].Target.Get();
		if (!TargetActor || !IsValid(TargetActor))
		{
			BurningTargets.RemoveAt(Index);
			continue;
		}

		FDamageEvent DamageEvent(UDamageType::StaticClass());
		TargetActor->TakeDamage(DamagePerTick, DamageEvent, GetInstigatorController(), this);
	}

	// Count down until the burn has fully expired, then stop the timer and clean up the projectile.
	BurnTimeRemaining -= DamageTickInterval;
	if (BurnTimeRemaining <= 0.f || BurningTargets.Num() == 0)
	{
		GetWorldTimerManager().ClearTimer(BurnTimerHandle);
		Destroy();
	}
}

void ATorchCPP::HandleImpact(const FVector& ImpactLocation)
{
	if (bImpactHandled)
	{
		// Ignore any extra collision events after the torch has already exploded once.
		return;
	}

	bImpactHandled = true;

	if (bApplyAOEOnImpact)
	{
		ApplyAreaDamage();
	}

	// Move the actor to the impact point so the lingering fire stays on the floor where the torch landed.
	SetActorLocation(ImpactLocation + FVector(0.f, 0.f, ImpactFlameHeightOffset));

	// Freeze the projectile in place so the lingering fire stays at the impact location.
	SetActorEnableCollision(false);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->Deactivate();
	
	// Hide the physical torch mesh so only the ground fire remains visible.
	ProjectileMesh->SetVisibility(false, true);

	// Start the impact fire only after the torch lands.
	FlameTrailEffect->SetRelativeLocation(FVector::ZeroVector);
	FlameTrailEffect->SetWorldScale3D(ImpactFlameScale);
	FlameTrailEffect->Activate(true);
	
	// If there is no DOT to manage, we can destroy the actor immediately after the impact burst.
	if (!bApplyBurningDOT || BurningTargets.Num() == 0)
	{
		Destroy();
	}
}

void ATorchCPP::OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	// Pawns usually trigger overlaps, so this path ensures the torch still explodes on living targets.
	FVector ImpactLocation = GetActorLocation();
	if (bFromSweep)
	{
		ImpactLocation = FVector(SweepResult.ImpactPoint);
	}
	HandleImpact(ImpactLocation);
}

void ATorchCPP::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	// Use the blocking hit location so impacts on walls and props sound correct.
	if (HitSounds)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSounds, Hit.ImpactPoint);
	}
	
	// World geometry usually triggers blocking hits, so this path covers walls, floors, and props.
	HandleImpact(Hit.ImpactPoint);
}
