// Fill out your copyright notice in the Description page of Project Settings.


#include "Weaponselector.h"
#include "ProjectileSpawner.h"
#include "Projectile_Base.h"

// Sets default values
AWeaponselector::AWeaponselector()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	// TEMP TESTING: this is 3 so key 3 can hold HeavyAxe during tuning.
	WeaponSlots.SetNum(HotbarSlotCount);
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

// Called when the game starts or when spawned
void AWeaponselector::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWeaponselector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeaponselector::InitializeWeaponSelector(AProjectileSpawner* InSpawner, TSubclassOf<AProjectile_Base> DefaultPrimaryWeaponClass,
	TSubclassOf<AProjectile_Base> DefaultSecondaryWeaponClass)
{
	Spawner = InSpawner;

	if (WeaponSlots.Num() < HotbarSlotCount)
	{
		WeaponSlots.SetNum(HotbarSlotCount);
	}

	// Seed the starting hotbar so weapon selection works before any pickups are collected.
	WeaponSlots[0] = DefaultPrimaryWeaponClass;
	WeaponSlots[1] = DefaultSecondaryWeaponClass;
	// TEMP TESTING: slot 2 is filled by the character with HeavyAxe after initialization.
	WeaponSlots[2] = nullptr;

	// Prefer slot 0 at startup, but fall back to slot 1 if only one default weapon exists.
	ActiveWeaponSlot = WeaponSlots[0] ? 0 : (WeaponSlots[1] ? 1 : INDEX_NONE);
	NextReplacementSlot = GetNextSlotToFill();
	if (NextReplacementSlot == INDEX_NONE)
	{
		// The hotbar is already full, so start the overwrite cycle at slot 0.
		NextReplacementSlot = 0;
	}

	ApplySelectedWeaponToSpawner();
}

bool AWeaponselector::AddWeaponToHotbar(TSubclassOf<AProjectile_Base> WeaponClass)
{
	if (!WeaponClass || WeaponSlots.Num() < 2)
	{
		return false;
	}

	// Treat re-picking an owned weapon as a selection change instead of storing duplicates.
	for (int32 SlotIndex = 0; SlotIndex < WeaponSlots.Num(); ++SlotIndex)
	{
		if (WeaponSlots[SlotIndex] == WeaponClass)
		{
			return SelectWeaponSlot(SlotIndex);
		}
	}

	const int32 EmptySlot = GetNextSlotToFill();
	const int32 SlotToUse = EmptySlot != INDEX_NONE ? EmptySlot : NextReplacementSlot;

	// Fill empty slots first, then rotate replacements across every active hotbar slot.
	WeaponSlots[SlotToUse] = WeaponClass;
	NextReplacementSlot = (SlotToUse + 1) % WeaponSlots.Num();
	return SelectWeaponSlot(SlotToUse);
}

bool AWeaponselector::SelectWeaponSlot(int32 SlotIndex)
{
	if (!WeaponSlots.IsValidIndex(SlotIndex) || !WeaponSlots[SlotIndex])
	{
		return false;
	}

	ActiveWeaponSlot = SlotIndex;
	ApplySelectedWeaponToSpawner();
	return true;
}

TSubclassOf<AProjectile_Base> AWeaponselector::GetWeaponInSlot(int32 SlotIndex) const
{
	return WeaponSlots.IsValidIndex(SlotIndex) ? WeaponSlots[SlotIndex] : nullptr;
}

int32 AWeaponselector::GetActiveWeaponSlot() const
{
	return ActiveWeaponSlot;
}

void AWeaponselector::ApplySelectedWeaponToSpawner()
{
	if (!Spawner || !WeaponSlots.IsValidIndex(ActiveWeaponSlot))
	{
		return;
	}

	if (WeaponSlots[ActiveWeaponSlot])
	{
		// The spawner stays generic; swapping this class is what changes the player's current weapon.
		Spawner->ProjectileActor = WeaponSlots[ActiveWeaponSlot];
	}
}

int32 AWeaponselector::GetNextSlotToFill() const
{
	for (int32 SlotIndex = 0; SlotIndex < WeaponSlots.Num(); ++SlotIndex)
	{
		if (!WeaponSlots[SlotIndex])
		{
			return SlotIndex;
		}
	}

	return INDEX_NONE;
}

