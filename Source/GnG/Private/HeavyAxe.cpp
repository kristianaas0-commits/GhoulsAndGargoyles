// Fill out your copyright notice in the Description page of Project Settings.


#include "HeavyAxe.h"

#include "Components/BoxComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"

AHeavyAxe::AHeavyAxe()
{
	// The axe should feel heavier than the default projectile, so it arcs more and travels slower.
	ProjectileMovement->ProjectileGravityScale = 5.f;
	ProjectileMovement->InitialSpeed = 2000.f;
	ProjectileMovement->MaxSpeed = 2500.f;
	
}

void AHeavyAxe::ApplyAreaDamage(const FVector& DamageOrigin)
{
	
	if (!GetWorld())
	{
		return;
	}
	
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(AoERadius);
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HeavyAxe), false, this);
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner());
	
	GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		DamageOrigin,
		FQuat::Identity,
		ObjectQueryParams,
		CollisionShape,
		QueryParams
	);
	
	// The overlap can return multiple components per actor, so track unique actors before damaging.
	TSet<AActor*> UniqueTargets;
	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* TargetActor = Result.GetActor();
		if (!TargetActor || TargetActor == GetOwner() || UniqueTargets.Contains(TargetActor))
		{
			continue;
		}
		
		UniqueTargets.Add(TargetActor);
		
		// Use the normal damage pipeline so gameplay hooks can respond consistently.
		FDamageEvent DamageEvent(UDamageType::StaticClass());
		TargetActor->TakeDamage(ImpactDamage,DamageEvent,GetInstigatorController(),this);
		
	}
	
}

void AHeavyAxe::HandleImpact(const FVector& ImpactLocation)
{
	if (bImpactHandled)
	{
		return;
	}
	
	bImpactHandled = true;
	
	if (bApplyAOEOnImpact)
	{
		ApplyAreaDamage(ImpactLocation);
	}
	
	// Stop all further collision and movement so the axe cannot keep traveling invisibly after impact.
	SetActorLocation(ImpactLocation);
	SetActorEnableCollision(false);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->Deactivate();
	ProjectileMesh->SetVisibility(false,true);

	Destroy();
}

void AHeavyAxe::OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}
	
	//Checks for pawns and other living
	FVector ImpactLocation = GetActorLocation();
	if (bFromSweep)
	{
		ImpactLocation = FVector(SweepResult.ImpactPoint);
	}
	HandleImpact(ImpactLocation);
}

void AHeavyAxe::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	if (HitSounds)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSounds, Hit.ImpactPoint);
	}
	
	//checks for walls,  floors and props
	HandleImpact(Hit.ImpactPoint);
	
}
