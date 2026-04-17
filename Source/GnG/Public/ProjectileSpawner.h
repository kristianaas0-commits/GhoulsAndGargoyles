// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileSpawner.generated.h"

class AProjectile_Base;
class UStaticMeshComponent;

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

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* SpawnerMesh;
	
	// Projectile class currently selected by the hotbar and used on the next fire input.
	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile_Base> ProjectileActor;

};
