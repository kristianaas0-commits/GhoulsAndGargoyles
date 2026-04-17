// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileSpawner.h"
#include "Projectile_Base.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"


// Sets default values
AProjectileSpawner::AProjectileSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	// The spawner is only an attachment/launch point, so it should never collide with the player.
	SpawnerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Spawner Mesh"));
	RootComponent = SpawnerMesh;
	SetActorEnableCollision(false);
	SpawnerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnerMesh->SetCollisionProfileName(TEXT("NoCollision"));
	SpawnerMesh->SetGenerateOverlapEvents(false);
	SpawnerMesh->CanCharacterStepUpOn = ECB_No;
	SpawnerMesh->SetSimulatePhysics(false);

}

// Called when the game starts or when spawned
void AProjectileSpawner::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AProjectileSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectileSpawner::Fire(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	if (!ProjectileActor || !GetWorld())
	{
		return;
	}

	// Forward the owner and instigator so damage can be attributed back to the player.
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = Cast<APawn>(GetOwner());
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn using the camera-based transform provided by the character rather than the spawner mesh rotation.
	GetWorld()->SpawnActor<AProjectile_Base>(ProjectileActor, SpawnLocation, SpawnRotation, SpawnParameters);
}
