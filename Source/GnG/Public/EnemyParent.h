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
	
	UPROPERTY(EditAnywhere)
	float DefaultHealth;
	
	UPROPERTY(BlueprintReadOnly)
    float CurrentHealth;
		
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
