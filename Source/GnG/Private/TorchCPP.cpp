// Fill out your copyright notice in the Description page of Project Settings.

#include "TorchCPP.h"
#include "Projectile_Base.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
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

void ATorchCPP::ApplyAreaDamage(float DamageAmount)
{
	if (!GetWorld() || DamageAmount <= 0.f)
	{
		return;
	}

	// The overlap query collects every actor close enough to the landed fire to be inside the torch blast.
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

		// Route all torch damage through Unreal's standard damage system.
		FDamageEvent DamageEvent(UDamageType::StaticClass());
		TargetActor->TakeDamage(DamageAmount, DamageEvent, GetInstigatorController(), this);
	}
}

void ATorchCPP::HandleBurnTick()
{
	// Each timer tick damages anyone currently standing in the fire.
	ApplyAreaDamage(DamagePerTick);

	// Count down until the fire has fully expired, then stop the timer and clean up the projectile.
	BurnTimeRemaining -= DamageTickInterval;
	if (BurnTimeRemaining <= 0.f)
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

	// Move the actor to the impact point before applying any AOE so the fire damages where it actually lands.
	SetActorLocation(ImpactLocation + FVector(0.f, 0.f, ImpactFlameHeightOffset));

	if (bApplyAOEOnImpact)
	{
		ApplyAreaDamage(ImpactDamage);
	}

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

	// Keep the fire around for a short time, optionally damaging actors that stay inside it.
	if (bApplyBurningDOT && DamageTickInterval > 0.f && DamagePerTick > 0.f && DamageDuration > 0.f)
	{
		BurnTimeRemaining = DamageDuration;
		GetWorldTimerManager().SetTimer(BurnTimerHandle, this, &ATorchCPP::HandleBurnTick, DamageTickInterval, true);
		return;
	}

	SetLifeSpan(FMath::Max(DamageDuration, 0.1f));
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
