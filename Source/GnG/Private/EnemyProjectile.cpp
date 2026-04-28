// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

// Sets default values
AEnemyProjectile::AEnemyProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Projectile Movement
	ProjectileMovement=CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	
	// Collision Sphere
	CollisionSphere=CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(1.f);// Set the radius
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // Sets the type of collision
	CollisionSphere->SetCollisionObjectType(ECC_Pawn); 
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	RootComponent = CollisionSphere;
	
	// OnBeginOverlap Event
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemyProjectile::OnOverlapBegin);
	
	//Mesh
	Mesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	
	Mesh->SetupAttachment(RootComponent); // Assign the mesh to the Collision Sphere
	
}	

// Called when the game starts or when spawned
void AEnemyProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// Destroys the projectile after a delau
	GetWorldTimerManager().SetTimer(
			DelayTimerHandle,
			this,
			&AEnemyProjectile::DestroySelf,
			5.f,
			false
			);
	
}

// Begin Overlap Event
void AEnemyProjectile::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult)
{
	// If the overlapping actor has a tag
	if (ActorHasTag(OtherActor("Player")))
	{
		// Applies damage
		UGameplayStatics::ApplyDamage(
			OtherActor,
			Damage,
			nullptr,
			this,
			UDamageType::StaticClass()
			);
		
		Destroy();// Destroys the projectile
	}
}

void AEnemyProjectile::DestroySelf()
{
	Destroy();
}

// Called every frame
void AEnemyProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

