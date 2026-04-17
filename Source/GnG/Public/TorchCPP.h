// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "Projectile_Base.h"
#include "TorchCPP.generated.h"

class UParticleSystemComponent;

USTRUCT()
struct FTorchBurnTarget
{
	GENERATED_BODY()

	// Stores one actor currently affected by the torch burn effect.
	UPROPERTY()
	TObjectPtr<AActor> Target = nullptr;
};

UCLASS()
class GNG_API ATorchCPP : public AProjectile_Base
{
	GENERATED_BODY()

public:
	// Projectile variant that explodes on impact and can leave behind a short burn effect.
	ATorchCPP();

protected:
	UFUNCTION()
	void ApplyAreaDamage();

	// Adds a target to the DOT list if it was caught in the impact blast.
	UFUNCTION()
	void ApplyBurningEffect(AActor* Target);

	// Repeating timer callback that applies DOT to every tracked burn target.
	UFUNCTION()
	void HandleBurnTick();

	// Centralized impact entry point so hit and overlap both trigger the same AOE/DOT behavior.
	void HandleImpact(const FVector& ImpactLocation);

	virtual void OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	virtual void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit) override;

	// Flame trail particle component attached to the torch projectile.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
	UParticleSystemComponent* FlameTrailEffect;

	// Scale to use while the torch is flying through the air.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch Effects")
	FVector FlameTrailScale = FVector(1.f, 1.f, 1.f);

	// Larger scale used after impact so the lingering ground fire reads as an AOE.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch Effects")
	FVector ImpactFlameScale = FVector(2.2f, 2.2f, 2.2f);

	// Small upward offset so the fire effect sits on top of the floor instead of clipping through it.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch Effects")
	float ImpactFlameHeightOffset = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch Damage")
	float ImpactDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch Damage")
	float DamagePerTick = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch Damage")
	float DamageDuration = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch Damage")
	float DamageTickInterval = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch AOE")
	float AOERadius = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch AOE")
	bool bApplyAOEOnImpact = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch Damage")
	bool bApplyBurningDOT = true;

	// Tracks actors that were inside the explosion so one timer can damage them over time.
	UPROPERTY()
	TArray<FTorchBurnTarget> BurningTargets;

	// A single repeating timer keeps the DOT logic simple and centralized.
	FTimerHandle BurnTimerHandle;

	// Counts down the total remaining burn duration after impact.
	float BurnTimeRemaining = 0.f;

	// Prevents overlap and hit events from triggering the explosion logic more than once.
	bool bImpactHandled = false;
};
