// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile_Base.h"
#include "HeavyAxe.generated.h"


UCLASS()
class GNG_API AHeavyAxe : public AProjectile_Base
{
	GENERATED_BODY()
	
public:
	// Heavier projectile variant that trades speed for an AOE impact burst.
	AHeavyAxe();
	
protected:
	UFUNCTION()
	void ApplyAreaDamage(const FVector& DamageOrigin);
	
	// Handles the one-shot heavy impact burst without any lingering DOT behavior.
	void HandleImpact(const FVector& ImpactLocation);
	
	virtual void OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
	virtual void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		FVector NormalImpulse, const FHitResult& Hit) override;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe Damage")
	float ImpactDamage = 30.f;
	
	// Radius used by the impact overlap query before applying AOE damage.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axe AOE")
	float AoERadius = 200.f;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Axe AOE")
	bool bApplyAOEOnImpact = true;
	
	// Prevents overlap and hit from both processing the same impact.
	bool bImpactHandled = false;
	
};
