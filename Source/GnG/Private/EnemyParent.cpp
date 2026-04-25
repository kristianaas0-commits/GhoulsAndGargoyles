// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyParent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AEnemyParent::AEnemyParent()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bIsDead = false;
	DefaultHealth = 100;
	CurrentHealth = DefaultHealth;
	
	StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));
}

// Called when the game starts or when spawned
void AEnemyParent::BeginPlay()
{
	Super::BeginPlay();
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
	
	// Set Hit Material
	GetMesh()->SetOverlayMaterial(HitMaterial);
	
	// Resets the Overlay material
	GetWorldTimerManager().SetTimer(
		DelayTimerHandle,
		this,
		&AEnemyParent::ResetMatrerial,
		0.2f,
		false)
	;
	
	// Checking if dead
	if (CurrentHealth <= 0)
	{
		// Score event dispatcher
		DeathEvent(KillingScore);
		
		// Stops the logic in the StateTree
		StateTreeComponent->StopLogic("");
		
		// Stops the movement
		GetCharacterMovement()->StopActiveMovement();
		
		// Tells the StateTree that it is dead
		bIsDead = true;
		
		// Play Death Sound
		UGameplayStatics::PlaySoundAtLocation(
			this,
			DeathSound,
			GetActorLocation()
			);
		
		// Play death Animation
		GetMesh()->PlayAnimation(DeathAnimation, false);
		
        // Destroys the Actor after the animation is finished
		GetWorldTimerManager().SetTimer(
			DelayTimerHandle,
			this,
			&AEnemyParent::DestroySelf,
			DeathAnimationDuration,
			false
			);
	}

	return AppliedDamage;
}

void AEnemyParent::DestroySelf()
{
	Destroy();
}

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

void AEnemyParent::DeathEvent(float Score)
{
	OnDeathForScore.Broadcast(Score);
}




