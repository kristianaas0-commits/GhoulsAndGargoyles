// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyParent.generated.h"

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
	void DestroySelf();
	
	UFUNCTION()
	void ResetMatrerial();
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	//Variables for delay
	FTimerHandle DelayTimerHandle;
	
	//Variables for When hit
	UPROPERTY(EditAnywhere, Category="Health")
	float DefaultHealth;
	
	UPROPERTY(BlueprintReadOnly, Category="Health")
    float CurrentHealth;
	
	UPROPERTY(EditAnywhere, Category = "Health")
	USoundBase* HitSound;
	
	UPROPERTY(EditAnywhere, Category="Health")
	UMaterial* HitMaterial;
	
	UPROPERTY(EditAnywhere, Category="Health")
	UMaterial* SeeThruMaterial;
	
	// Variables for when character dies
	UPROPERTY(EditAnywhere, Category="Death")
	UAnimMontage* DeathAnimation;

	UPROPERTY(EditAnywhere, Category="Death")
	float DeathAnimationDuration;
	
	UPROPERTY(EditAnywhere, Category = "Death")
    USoundBase* DeathSound;
	
	UPROPERTY(EditAnywhere, Category="Death")
	bool bIsDead;

	UPROPERTY(EditAnywhere, Category="Death")
    float KillingScore;

};
