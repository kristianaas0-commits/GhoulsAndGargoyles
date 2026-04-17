// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickupable_Base.h"
#include "CoinPickup.generated.h"

UCLASS()
class GNG_API ACoinPickup : public APickupable_Base
{
	GENERATED_BODY()

public:
	// Preconfigures the shared pickup base to behave like a score pickup.
	ACoinPickup();
};
