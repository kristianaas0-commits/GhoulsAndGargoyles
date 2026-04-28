// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile_Base.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AudioDevice.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/DamageType.h"

namespace
{
	void PlaySoundAtWorldLocation(AActor* SourceActor, USoundBase* Sound, const FVector& Location)
	{
		if (!SourceActor || !Sound)
		{
			return;
		}

		if (UWorld* World = SourceActor->GetWorld())
		{
			if (FAudioDevice* AudioDevice = World->GetAudioDeviceRaw())
			{
				AudioDevice->PlaySoundAtLocation(Sound, World, 1.f, 1.f, 0.f, Location, FRotator::ZeroRotator);
			}
		}
	}
}

// Sets default values
AProjectile_Base::AProjectile_Base()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Use a thin box so the collision better matches a flat stick-like projectile.
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	RootComponent = CollisionBox;
	CollisionBox->InitBoxExtent(FVector(40.f, 4.f, 4.f));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionBox->SetGenerateOverlapEvents(true);
	CollisionBox->SetNotifyRigidBodyCollision(true);
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AProjectile_Base::OnProjectileOverlap);
	CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile_Base::OnProjectileHit);

	// The mesh is visual only and can be swapped in child Blueprints.
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetGenerateOverlapEvents(false);

	// Movement defaults can be overridden per child Blueprint.
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
	ProjectileMovement->UpdatedComponent = CollisionBox;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3500.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 3.f;
	
	
	
}

// Called when the game starts or when spawned
void AProjectile_Base::BeginPlay()
{
	Super::BeginPlay();

	// Ignore the actor that fired the projectile so it does not hit immediately.
	if (AActor* OwnerActor = GetOwner())
	{
		CollisionBox->IgnoreActorWhenMoving(OwnerActor, true);
		ProjectileMesh->IgnoreActorWhenMoving(OwnerActor, true);
	}
	
}

// Called every frame
void AProjectile_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//Check whether or not it hit something and if it should apply damage
void AProjectile_Base::OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	// Route damage through Unreal's standard damage system so AnyDamage and similar hooks still fire.
	AController* InstigatorController = nullptr;
	if (APawn* InstigatorPawn = GetInstigator())
	{
		InstigatorController = InstigatorPawn->GetController();
	}

	FDamageEvent DamageEvent(UDamageType::StaticClass());
	OtherActor->TakeDamage(Damage, DamageEvent, InstigatorController, this);

	// Play the configured throw sound before the projectile destroys itself.
	if (HitSounds)
	{
		PlaySoundAtWorldLocation(this, HitSounds, SweepResult.ImpactPoint);
	}

	Destroy();
}

//Checks if the actor is the player or if its part of the scene before it applies damage or if it should destroy the itself
void AProjectile_Base::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	// Use the blocking hit location so impacts on walls and props sound correct.
	if (HitSounds)
	{
		PlaySoundAtWorldLocation(this, HitSounds, Hit.ImpactPoint);
	}

	Destroy();
}


