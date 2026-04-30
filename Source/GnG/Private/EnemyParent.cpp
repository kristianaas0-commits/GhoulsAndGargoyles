// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyParent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AEnemyParent::AEnemyParent()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Setting some variables
	bIsDead = false;
	CurrentHealth = 100;
	
	StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent")); // Creates the StateTree subobject
}

// Called when the game starts or when spawned
void AEnemyParent::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = DefaultHealth;
}

float AEnemyParent::TakeDamage(
	float DamageAmount,
	const FDamageEvent& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{
	// All weapon projectiles call Unreal's TakeDamage pipeline, so this is the single place enemies lose health.
	const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (AppliedDamage <= 0.f)
	{
		// Ignore zero-damage hits so harmless overlaps do not change enemy state.
		return 0.f;
	}
	
	// Use the final applied damage value in case future damage modifiers change the incoming amount.
	CurrentHealth = FMath::Clamp(CurrentHealth - AppliedDamage, 0.0f, DefaultHealth);
	
	// Play sound when hit
	UGameplayStatics::PlaySoundAtLocation(
		this,
		HitSound,
		GetActorLocation()
	);
	
	GetMesh()->SetOverlayMaterial(HitMaterial); // Applying the overlay material Hit Material
	
	// Removes the overlay material after a delay
	GetWorldTimerManager().SetTimer(
		DelayTimerHandle,
		this,
		&AEnemyParent::ResetMatrerial,
		0.2f,
		false)
	;
	
	// Checking if dead
	if (CurrentHealth <= 0 && bIsDead==false)
	{
		DeathEvent(KillingScore, bIsCyclops); // Calling Score Event function
		
		StateTreeComponent->StopLogic(""); // Stops the logic in the StateTree
		
		GetCharacterMovement()->DisableMovement(); // Stops the movement
		
		bIsDead = true; 
		
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		// Play Death Sound
		UGameplayStatics::PlaySoundAtLocation(
			this,
			DeathSound,
			GetActorLocation()
			);
		
		GetMesh()->PlayAnimation(DeathAnimation, false); // Play death Animation
		
        // Destroys the Actor after the animation is finished
		GetWorldTimerManager().SetTimer(
			DestroyTimerHandle,
			this,
			&AEnemyParent::DestroySelf,
			DeathAnimationDuration,
			false
			);
	}

	return AppliedDamage;
}

// Destroys the actor
void AEnemyParent::DestroySelf()
{
	Destroy();
}

// Removes the overlay material
void AEnemyParent::ResetMatrerial()
{
	GetMesh()->SetOverlayMaterial(SeeThruMaterial);
}

// Called every frame
void AEnemyParent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AEnemyParent::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// Event broadcaster for updating the score
void AEnemyParent::DeathEvent(float inScore, bool bInIsCyclopsValue)
{
	OnDeathForScore.Broadcast(inScore, bInIsCyclopsValue);
	
}




