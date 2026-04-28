// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/StateTreeComponent.h"
#include "EnemyParent.generated.h"

// Declaring the event that updates the score when an enemy dies.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeathForScore, float, Score, bool, bIsCyclops);

UCLASS()
class GNG_API AEnemyParent : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyParent();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Central damage hook used by all weapons that call Unreal's standard TakeDamage pipeline.
	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser
	) override;

	UFUNCTION()
	void DestroySelf(); // Function called after a delay, destroys self
	
	UFUNCTION()
	void ResetMatrerial(); // Function called after a delay, removes the overlay matrial
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	//Variables for delay
	FTimerHandle DelayTimerHandle;
	
	/*
	 * StateTree
	 */
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI")
	UStateTreeComponent* StateTreeComponent; // State Tree
	
	/*
	 * Variables for When hit
	*/
	
	UPROPERTY(EditAnywhere, Category="Health") 
	float DefaultHealth; //Max health
	
	UPROPERTY(BlueprintReadOnly, Category="Health")
    float CurrentHealth; // Current health
	
	UPROPERTY(EditAnywhere, Category = "Health")
	USoundBase* HitSound; // Sound that plays if enemy get hit
	
	UPROPERTY(EditAnywhere, Category="Health")
	UMaterial* HitMaterial; // Overlay material to visually represent when the enemy get hit
	
	UPROPERTY(EditAnywhere, Category="Health")
	UMaterial* SeeThruMaterial; // Needed for removing the overlay material
	
	
	/*
	 * Variables for when the enemy dies	
	*/
	
	UPROPERTY(BlueprintAssignable, Category = "Death")
	FOnDeathForScore OnDeathForScore; // Event
	
	UFUNCTION(BlueprintCallable, Category = "Death")
	void DeathEvent(float Score, bool bIsCyclopsValue); // Function to call Event broadcast
	
	UPROPERTY(EditAnywhere, Category="Death")
	UAnimSequence* DeathAnimation; // Death animation

	UPROPERTY(EditAnywhere, Category="Death")
	float DeathAnimationDuration; // Wait between the damage being applied and the actor being destroyed
	
	UPROPERTY(EditAnywhere, Category = "Death")
    USoundBase* DeathSound; // Death sound
	
	UPROPERTY(EditAnywhere, Category="Death")
	bool bIsDead; // Bool 

	UPROPERTY(EditAnywhere, Category="Death")
    float KillingScore; // Score gained for killing the enemy
	
	UPROPERTY(EditAnywhere, Category="Death")
	bool bIsCyclops;
};
