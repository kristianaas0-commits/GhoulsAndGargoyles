// Fill out your copyright notice in the Description page of Project Settings.



#include "CPP_EnemySpawner.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

#include "CPP_EnemySpawner.h"
// Sets default values
ACPP_EnemySpawner::ACPP_EnemySpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	

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
	ACharacter* myCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	myCharacter->GetActorLocation();
	
	DistanceBetweenPlayerAndSpawner = GetHorizontalDistanceTo(myCharacter);
	
	if (DistanceBetweenPlayerAndSpawner <= DistanceBeforeStartSpawning)
	{
		if (CanSpawnEnemy == true)
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
