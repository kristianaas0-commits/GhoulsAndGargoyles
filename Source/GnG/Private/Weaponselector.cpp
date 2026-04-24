// Fill out your copyright notice in the Description page of Project Settings.


#include "Weaponselector.h"
#include "ProjectileSpawner.h"
#include "Projectile_Base.h"

// Sets default values
AWeaponselector::AWeaponselector()
{
	PrimaryActorTick.bCanEverTick = false;
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

void AWeaponselector::InitializeWeaponSelector(AProjectileSpawner* InSpawner, TSubclassOf<AProjectile_Base> PrimaryWeaponClass,
	TSubclassOf<AProjectile_Base> SecondaryWeaponClass, TSubclassOf<AProjectile_Base> TertiaryWeaponClass)
{
	Spawner = InSpawner;

	if (WeaponSlots.Num() < HotbarSlotCount)
	{
		WeaponSlots.SetNum(HotbarSlotCount);
	}

	WeaponSlots[0] = PrimaryWeaponClass;
	WeaponSlots[1] = SecondaryWeaponClass;
	WeaponSlots[2] = TertiaryWeaponClass;

	ActiveWeaponSlot = WeaponSlots[0] ? 0 : (WeaponSlots[1] ? 1 : (WeaponSlots[2] ? 2 : INDEX_NONE));

	ApplySelectedWeaponToSpawner();
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


