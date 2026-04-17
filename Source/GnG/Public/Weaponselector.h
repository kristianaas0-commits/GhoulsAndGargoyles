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

	// Connects the selector to the player's spawner and seeds the initial hotbar state.
	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	void InitializeWeaponSelector(AProjectileSpawner* InSpawner, TSubclassOf<AProjectile_Base> DefaultPrimaryWeaponClass,
		TSubclassOf<AProjectile_Base> DefaultSecondaryWeaponClass);

	// Adds a weapon to the next free slot, or rotates replacements once the hotbar is full.
	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	bool AddWeaponToHotbar(TSubclassOf<AProjectile_Base> WeaponClass);

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

	// When all slots are full, new pickups overwrite this slot and then advance.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hotbar")
	int32 NextReplacementSlot = 0;

protected:
	// Shared projectile spawner that actually fires the currently selected weapon class.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Hotbar")
	TObjectPtr<AProjectileSpawner> Spawner = nullptr;

private:
	// TEMP TESTING: increase this back to 2 when removing the temporary HeavyAxe slot.
	static constexpr int32 HotbarSlotCount = 3;

	// Pushes the active slot's weapon class into the projectile spawner.
	void ApplySelectedWeaponToSpawner();

	// Returns the first empty slot, or INDEX_NONE when every slot is occupied.
	int32 GetNextSlotToFill() const;

};
