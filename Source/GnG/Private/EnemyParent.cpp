// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyParent.h"
#include "Engine/DamageEvents.h"

// Sets default values
AEnemyParent::AEnemyParent()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	DefaultHealth = 100;
	CurrentHealth = DefaultHealth;
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
	
	if (CurrentHealth <= 0)
	{
		// Removing the actor here makes death immediate for all current weapon types.
		Destroy();
	}

	return AppliedDamage;
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




