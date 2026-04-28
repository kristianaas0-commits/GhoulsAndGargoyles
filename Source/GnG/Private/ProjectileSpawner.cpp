// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileSpawner.h"
#include "Projectile_Base.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"


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

	DefaultFireConfig.FireCooldownSeconds = 0.0f;
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

const FWeaponFireConfig& AProjectileSpawner::GetFireConfigFor(TSubclassOf<AProjectile_Base> WeaponClass) const
{
	if (const FWeaponFireConfig* FoundConfig = WeaponFireConfigs.Find(WeaponClass))
	{
		return *FoundConfig;
	}

	return DefaultFireConfig;
}

bool AProjectileSpawner::CanFireWeapon(TSubclassOf<AProjectile_Base> WeaponClass) const
{
	if (!WeaponClass)
	{
		return false;
	}

	if (const bool* FoundCanFire = WeaponCanFireStates.Find(WeaponClass))
	{
		return *FoundCanFire;
	}

	return true;
}

void AProjectileSpawner::EnsureWeaponFireStateInitialized(TSubclassOf<AProjectile_Base> WeaponClass)
{
	if (!WeaponClass || WeaponCanFireStates.Contains(WeaponClass))
	{
		return;
	}

	// New weapon classes start ready to fire the first time the player switches to them.
	WeaponCanFireStates.Add(WeaponClass, true);
}

void AProjectileSpawner::StartFireCooldown(TSubclassOf<AProjectile_Base> WeaponClass)
{
	if (!WeaponClass || !GetWorld())
	{
		return;
	}

	const float CooldownSeconds = FMath::Max(0.0f, GetFireConfigFor(WeaponClass).FireCooldownSeconds);
	if (CooldownSeconds <= 0.0f)
	{
		WeaponCanFireStates.FindOrAdd(WeaponClass) = true;
		return;
	}

	// Mark just this weapon as blocked; swapping weapons should not pause or reset other cooldowns.
	WeaponCanFireStates.FindOrAdd(WeaponClass) = false;

	FTimerHandle& TimerHandle = WeaponCooldownTimerHandles.FindOrAdd(WeaponClass);
	GetWorldTimerManager().ClearTimer(TimerHandle);
	// When the timer completes, this weapon class becomes available again.
	GetWorldTimerManager().SetTimer(
		TimerHandle,
		FTimerDelegate::CreateUObject(this, &AProjectileSpawner::ResetWeaponCanFire, WeaponClass),
		CooldownSeconds,
		false);
}

void AProjectileSpawner::ResetWeaponCanFire(TSubclassOf<AProjectile_Base> WeaponClass)
{
	if (!WeaponClass)
	{
		return;
	}

	WeaponCanFireStates.FindOrAdd(WeaponClass) = true;
}

bool AProjectileSpawner::CanFireCurrentWeapon() const
{
	return CanFireWeapon(ProjectileActor);
}

AProjectile_Base* AProjectileSpawner::Fire(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	// Return the spawned projectile so callers can react only when a shot really happened.
	if (!ProjectileActor || !GetWorld())
	{
		return nullptr;
	}

	EnsureWeaponFireStateInitialized(ProjectileActor);
	// Cooldown blocks the shot entirely, so callers also get nullptr here.
	if (!CanFireWeapon(ProjectileActor))
	{
		return nullptr;
	}

	// Forward the owner and instigator so damage can be attributed back to the player.
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = Cast<APawn>(GetOwner());
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn using the camera-based transform provided by the character rather than the spawner mesh rotation.
	AProjectile_Base* SpawnedProjectile =
		GetWorld()->SpawnActor<AProjectile_Base>(ProjectileActor, SpawnLocation, SpawnRotation, SpawnParameters);
	if (!SpawnedProjectile)
	{
		return nullptr;
	}

	// Only start cooldown after spawn succeeds.
	StartFireCooldown(ProjectileActor);
	return SpawnedProjectile;
}

bool AProjectileSpawner::TryFire(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	// Preserve the bool API by treating a valid spawned projectile as a successful fire.
	return Fire(SpawnLocation, SpawnRotation) != nullptr;
}
