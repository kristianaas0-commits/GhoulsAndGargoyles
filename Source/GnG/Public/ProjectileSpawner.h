// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "ProjectileSpawner.generated.h"

class AProjectile_Base;
class UStaticMeshComponent;

USTRUCT(BlueprintType)
struct FWeaponMagazineConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 MagazineSize = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float ReloadTimeSeconds = 1.0f;
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
	void Fire(const FVector& SpawnLocation, const FRotator& SpawnRotation);

	// Same as Fire, but returns whether a projectile was spawned (not reloading / not empty).
	UFUNCTION(BlueprintCallable, Category = "Weapon|Magazine")
	bool TryFire(const FVector& SpawnLocation, const FRotator& SpawnRotation);

	// Remaining ammo for the currently selected projectile class.
	UFUNCTION(BlueprintPure, Category = "Weapon|Magazine")
	int32 GetAmmoRemaining() const;

	// Magazine size for the currently selected projectile class.
	UFUNCTION(BlueprintPure, Category = "Weapon|Magazine")
	int32 GetMagazineSize() const;

	// True while the currently selected weapon is reloading.
	UFUNCTION(BlueprintPure, Category = "Weapon|Magazine")
	bool IsReloading() const;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* SpawnerMesh;
	
	// Projectile class currently selected by the hotbar and used on the next fire input.
	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile_Base> ProjectileActor;

	// Per-weapon magazine settings. If a weapon class is not present, defaults are used.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Magazine")
	TMap<TSubclassOf<AProjectile_Base>, FWeaponMagazineConfig> MagazineConfigs;

	// Fallback used when the projectile class has no entry in MagazineConfigs.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Magazine")
	FWeaponMagazineConfig DefaultMagazineConfig;

protected:
	// Ammo lives on the persistent spawner, keyed by weapon class, so it survives projectile spawns.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon|Magazine")
	TMap<TSubclassOf<AProjectile_Base>, int32> AmmoRemainingByWeapon;

	// Reload is tracked per weapon class, which lets each slot keep its own state when the player swaps weapons.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon|Magazine")
	TSet<TSubclassOf<AProjectile_Base>> ReloadingWeapons;

	UPROPERTY()
	TMap<TSubclassOf<AProjectile_Base>, FTimerHandle> ReloadTimerHandles;

	const FWeaponMagazineConfig& GetMagazineConfigFor(TSubclassOf<AProjectile_Base> WeaponClass) const;
	void EnsureWeaponStateInitialized(TSubclassOf<AProjectile_Base> WeaponClass);
	void StartReload(TSubclassOf<AProjectile_Base> WeaponClass);
	void FinishReload(TSubclassOf<AProjectile_Base> WeaponClass);
};
