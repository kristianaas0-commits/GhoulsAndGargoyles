// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPP_EnemySpawner.generated.h"

class UStaticMeshComponent;

UCLASS()
class GNG_API ACPP_EnemySpawner : public AActor
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> EnemyClass;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMesh;
	
public:	
	// Sets default values for this actor's properties
	ACPP_EnemySpawner();
	UFUNCTION()
	void SpawnWait();
	UFUNCTION()
	void SpawnEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	FTimerHandle SpawnTimerHandle;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
private:
	float DistanceBeforeStartSpawning = 500;
	bool CanSpawnEnemy = true;
	float DistanceBetweenPlayerAndSpawner;
	float SpawnCooldown = 5.0f;

};
