// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weaponselector.generated.h"

class AProjectileSpawner;
class AProjectile_Base;

UCLASS()
class GNG_API AWeaponselector : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponselector();

	// Connects the selector to the player's spawner and seeds the fixed hotbar layout.
	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	void InitializeWeaponSelector(AProjectileSpawner* InSpawner, TSubclassOf<AProjectile_Base> PrimaryWeaponClass,
		TSubclassOf<AProjectile_Base> SecondaryWeaponClass, TSubclassOf<AProjectile_Base> TertiaryWeaponClass);

	// Makes the requested slot active and updates the shared projectile spawner.
	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	bool SelectWeaponSlot(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Hotbar")
	TSubclassOf<AProjectile_Base> GetWeaponInSlot(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Hotbar")
	int32 GetActiveWeaponSlot() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Hotbar slots that hold projectile classes for the shared spawner.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hotbar")
	TArray<TSubclassOf<AProjectile_Base>> WeaponSlots;

	// Active slot used when firing. 0 = key 1, 1 = key 2, 2 = key 3 during temporary testing.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hotbar")
	int32 ActiveWeaponSlot = INDEX_NONE;

protected:
	// Shared projectile spawner that actually fires the currently selected weapon class.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Hotbar")
	TObjectPtr<AProjectileSpawner> Spawner = nullptr;

private:
	// Fixed hotbar slot count: 1 = Lance, 2 = Torch, 3 = HeavyAxe.
	static constexpr int32 HotbarSlotCount = 3;

	// Pushes the active slot's weapon class into the projectile spawner.
	void ApplySelectedWeaponToSpawner();

};
