// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
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
	RootComponent = CollisionSphere;
	
	CollisionSphere->InitSphereRadius(20.f);// Set the radius
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // Sets the type of collision
	CollisionSphere->SetCollisionObjectType(ECC_GameTraceChannel1); // Collision channel
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block); // Default collision type is to block the projectile
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block); // Collision with static object is blocked
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // Collision with pawns is overlap
	CollisionSphere->SetGenerateOverlapEvents(true); // Used to signal overlap event for OnOverlapBegin
	CollisionSphere->SetNotifyRigidBodyCollision(true); // Used to signal hit events for ProjectileHit
	
	
	
	// OnBeginOverlap Event
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemyProjectile::OnOverlapBegin); // Overlap Event
	CollisionSphere->OnComponentHit.AddDynamic(this, &AEnemyProjectile::OnProjectileHit); // Hit Event
	ProjectileMovement->UpdatedComponent = Mesh; // Sets the root
	
	//Mesh
	Mesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // sets the collision for the mesh to no collision
	Mesh->SetGenerateOverlapEvents(false); // Does not create overlap events
	Mesh->SetupAttachment(RootComponent); // Assign the mesh to the Collision Sphere
	
}	

// Called when the game starts or when spawned
void AEnemyProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// Collision ignores the shooter
	if (AActor* OwnerActor=GetOwner())
	{
		CollisionSphere->IgnoreActorWhenMoving(OwnerActor, true);
		Mesh->IgnoreActorWhenMoving(OwnerActor, true);
	}
	
	// Destroys the projectile after a delaY
	GetWorldTimerManager().SetTimer(
			DelayTimerHandle,
			this,
			&AEnemyProjectile::DestroySelf,
			5.f,
			false
			);
	
	
}

// Called every frame
void AEnemyProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}
	
	// If the overlapping actor is the player character
	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (OtherActor && PlayerChar && OtherActor == PlayerChar)
	{
		// Applies damage
		UGameplayStatics::ApplyDamage(
			OtherActor,
			Damage,
			nullptr,
			this,
			UDamageType::StaticClass()
			);
	}
	
	Destroy(); // Destroys the projectile
}

void AEnemyProjectile::OnProjectileHit(
	UPrimitiveComponent* HitComponent, 
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp, 
	FVector NormalImpulse, 
	const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}
	
	Destroy();
}

void AEnemyProjectile::DestroySelf()
{
	Destroy();
}