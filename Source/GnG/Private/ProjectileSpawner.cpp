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

	// Reasonable fallback defaults; override per weapon via MagazineConfigs in the editor/Blueprint.
	DefaultMagazineConfig.MagazineSize = 1;
	DefaultMagazineConfig.ReloadTimeSeconds = 1.0f;
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

const FWeaponMagazineConfig& AProjectileSpawner::GetMagazineConfigFor(TSubclassOf<AProjectile_Base> WeaponClass) const
{
	if (const FWeaponMagazineConfig* FoundConfig = MagazineConfigs.Find(WeaponClass))
	{
		return *FoundConfig;
	}

	return DefaultMagazineConfig;
}

void AProjectileSpawner::EnsureWeaponStateInitialized(TSubclassOf<AProjectile_Base> WeaponClass)
{
	if (!WeaponClass)
	{
		return;
	}

	if (AmmoRemainingByWeapon.Contains(WeaponClass))
	{
		return;
	}

	const FWeaponMagazineConfig& Config = GetMagazineConfigFor(WeaponClass);
	// First time this weapon is selected, seed its runtime ammo from the editor-configured magazine size.
	AmmoRemainingByWeapon.Add(WeaponClass, FMath::Max(0, Config.MagazineSize));
}

void AProjectileSpawner::StartReload(TSubclassOf<AProjectile_Base> WeaponClass)
{
	if (!WeaponClass || ReloadingWeapons.Contains(WeaponClass) || !GetWorld())
	{
		return;
	}

	const FWeaponMagazineConfig& Config = GetMagazineConfigFor(WeaponClass);
	const float ReloadDelay = FMath::Max(0.0f, Config.ReloadTimeSeconds);

	ReloadingWeapons.Add(WeaponClass);

	FTimerHandle& Handle = ReloadTimerHandles.FindOrAdd(WeaponClass);
	GetWorldTimerManager().ClearTimer(Handle);
	// Reload completes asynchronously so fire input can simply query ReloadingWeapons and bail out.
	GetWorldTimerManager().SetTimer(
		Handle,
		FTimerDelegate::CreateUObject(this, &AProjectileSpawner::FinishReload, WeaponClass),
		ReloadDelay,
		false);
}

void AProjectileSpawner::FinishReload(TSubclassOf<AProjectile_Base> WeaponClass)
{
	if (!WeaponClass)
	{
		return;
	}

	const FWeaponMagazineConfig& Config = GetMagazineConfigFor(WeaponClass);
	AmmoRemainingByWeapon.FindOrAdd(WeaponClass) = FMath::Max(0, Config.MagazineSize);
	ReloadingWeapons.Remove(WeaponClass);
}

int32 AProjectileSpawner::GetAmmoRemaining() const
{
	if (!ProjectileActor)
	{
		return 0;
	}

	if (const int32* FoundAmmo = AmmoRemainingByWeapon.Find(ProjectileActor))
	{
		return *FoundAmmo;
	}

	// Not initialized yet; report full mag size so UI doesn't flash 0 on begin play.
	return FMath::Max(0, GetMagazineConfigFor(ProjectileActor).MagazineSize);
}

int32 AProjectileSpawner::GetMagazineSize() const
{
	return ProjectileActor ? FMath::Max(0, GetMagazineConfigFor(ProjectileActor).MagazineSize) : 0;
}

bool AProjectileSpawner::IsReloading() const
{
	return ProjectileActor ? ReloadingWeapons.Contains(ProjectileActor) : false;
}

void AProjectileSpawner::Fire(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	TryFire(SpawnLocation, SpawnRotation);
}

bool AProjectileSpawner::TryFire(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	if (!ProjectileActor || !GetWorld())
	{
		return false;
	}

	EnsureWeaponStateInitialized(ProjectileActor);

	if (ReloadingWeapons.Contains(ProjectileActor))
	{
		return false;
	}

	int32& AmmoRemaining = AmmoRemainingByWeapon.FindOrAdd(ProjectileActor);
	if (AmmoRemaining <= 0)
	{
		// Hitting an empty magazine starts reload on the current weapon instead of spawning anything.
		StartReload(ProjectileActor);
		return false;
	}

	// Forward the owner and instigator so damage can be attributed back to the player.
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = Cast<APawn>(GetOwner());
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn using the camera-based transform provided by the character rather than the spawner mesh rotation.
	GetWorld()->SpawnActor<AProjectile_Base>(ProjectileActor, SpawnLocation, SpawnRotation, SpawnParameters);

	AmmoRemaining = FMath::Max(0, AmmoRemaining - 1);
	if (AmmoRemaining <= 0)
	{
		// The shot that empties the mag is still allowed; reload begins immediately after it leaves the spawner.
		StartReload(ProjectileActor);
	}

	return true;
}
