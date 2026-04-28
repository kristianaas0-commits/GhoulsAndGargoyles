// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "ProjectileSpawner.generated.h"

class AProjectile_Base;
class UStaticMeshComponent;

USTRUCT(BlueprintType)
struct FWeaponFireConfig
{
	GENERATED_BODY()

	// Minimum time between shots for this weapon class.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float FireCooldownSeconds = 0.0f;
};

UCLASS()
class GNG_API AProjectileSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectileSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Spawns the configured projectile class using the transform supplied by the character.
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	AProjectile_Base* Fire(const FVector& SpawnLocation, const FRotator& SpawnRotation);

	// Same as Fire, but returns false when the current weapon is still on cooldown.
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool TryFire(const FVector& SpawnLocation, const FRotator& SpawnRotation);

	// Returns whether the currently selected weapon is allowed to fire right now.
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool CanFireCurrentWeapon() const;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* SpawnerMesh;
	
	// Projectile class currently selected by the hotbar and used on the next fire input.
	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile_Base> ProjectileActor;

	// Per-weapon fire rate settings keyed by projectile class.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TMap<TSubclassOf<AProjectile_Base>, FWeaponFireConfig> WeaponFireConfigs;

	// Used when a weapon class has no specific entry in WeaponFireConfigs.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FWeaponFireConfig DefaultFireConfig;

protected:
	// Cooldown state lives on the spawner because the spawner persists while each projectile does not.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon")
	TMap<TSubclassOf<AProjectile_Base>, bool> WeaponCanFireStates;

	// Each weapon class gets its own timer handle so Lance, Torch, and Axe can cool down independently.
	UPROPERTY()
	TMap<TSubclassOf<AProjectile_Base>, FTimerHandle> WeaponCooldownTimerHandles;

	const FWeaponFireConfig& GetFireConfigFor(TSubclassOf<AProjectile_Base> WeaponClass) const;
	bool CanFireWeapon(TSubclassOf<AProjectile_Base> WeaponClass) const;
	void EnsureWeaponFireStateInitialized(TSubclassOf<AProjectile_Base> WeaponClass);
	void StartFireCooldown(TSubclassOf<AProjectile_Base> WeaponClass);
	void ResetWeaponCanFire(TSubclassOf<AProjectile_Base> WeaponClass);

};
