// Fill out your copyright notice in the Description page of Project Settings.

#include "CoinPickup.h"

ACoinPickup::ACoinPickup()
{
	// Coins always use the score reward path.
	PickupRewardType = EPickupRewardType::Score;
	// Child Blueprints can override this, but 1 is a safe default for quick placement.
	ScoreAmount = 100;
}
