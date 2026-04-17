// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponPickup.h"

AWeaponPickup::AWeaponPickup()
{
	// Weapon drops always use the weapon reward path.
	PickupRewardType = EPickupRewardType::Weapon;
}
