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

	// Score amount awarded when the player overlaps this pickup.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (ClampMin = "0"))
	float ScoreAmount = 100;
	
	// Shared overlap callback for score pickups.
	UFUNCTION()
	void OnPlayerEnterPickupBox(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Returns true only when the pickup should be consumed.
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	virtual bool HandleScorePickup(AJarl_ThirdPersonCharacter_CPP* PlayerCharacter);
};
