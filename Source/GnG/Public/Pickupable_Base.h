// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickupable_Base.generated.h"

class AJarl_ThirdPersonCharacter_CPP;
class AProjectile_Base;
class UPrimitiveComponent;
class USceneComponent;
class UShapeComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EPickupRewardType : uint8
{
	// Adds score to the player through Jarl_ThirdPersonCharacter_CPP::UpdateScore.
	Score UMETA(DisplayName = "Score"),
	// Grants or selects a weapon through Jarl_ThirdPersonCharacter_CPP::AddWeaponToHotbar.
	Weapon UMETA(DisplayName = "Weapon")
};

UCLASS()
class GNG_API APickupable_Base : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickupable_Base();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	
	/**
	 * Static Mesh for the pickup
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	USceneComponent* PickUpRoot;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	UStaticMeshComponent* PickUpMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	UShapeComponent* PickUpShape;

	// Chooses which reward path runs when the player overlaps this pickup.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	EPickupRewardType PickupRewardType = EPickupRewardType::Score;

	// Score amount awarded when this pickup is configured as a score pickup.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (ClampMin = "0"))
	int32 ScoreAmount = 100;

	// Weapon class granted when this pickup is configured as a weapon pickup.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	TSubclassOf<AProjectile_Base> WeaponClass;
	
	// Shared overlap callback used by both coins and weapon drops.
	UFUNCTION()
	void OnPlayerEnterPickupBox(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Score-specific reward path. Returns true only when the pickup should be consumed.
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	virtual bool HandleScorePickup(AJarl_ThirdPersonCharacter_CPP* PlayerCharacter);

	// Weapon-specific reward path. Returns true only when the pickup should be consumed.
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	virtual bool HandleWeaponPickup(AJarl_ThirdPersonCharacter_CPP* PlayerCharacter);
};
