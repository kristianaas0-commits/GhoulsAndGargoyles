// Fill out your copyright notice in the Description page of Project Settings.

#include "CPP_EnemySpawner.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

// Sets default values
ACPP_EnemySpawner::ACPP_EnemySpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);
}

// Called when the game starts or when spawned
void ACPP_EnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACPP_EnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ACharacter* MyCharacter = nullptr;
	if (APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		MyCharacter = PlayerController->GetCharacter();
	}

	if (!MyCharacter)
	{
		return;
	}

	DistanceBetweenPlayerAndSpawner = GetHorizontalDistanceTo(MyCharacter);
	
	if (DistanceBetweenPlayerAndSpawner <= DistanceBeforeStartSpawning)
	{
		if (CanSpawnEnemy)
		{
			SpawnWait();
		}
	}

}

void ACPP_EnemySpawner::SpawnWait()
{
	CanSpawnEnemy = false;
	GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &ACPP_EnemySpawner::SpawnEnemy, SpawnCooldown, false);
	CanSpawnEnemy = true;
	
}

void ACPP_EnemySpawner::SpawnEnemy()
{
	FVector SpawnLocation = StaticMesh->GetSocketLocation(NAME_None);
	FRotator SpawnRotation = FRotator::ZeroRotator;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = 
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	AActor* SpawnedEnemy = GetWorld()->SpawnActor<AActor>(EnemyClass, SpawnLocation, SpawnRotation, SpawnParams);
}
