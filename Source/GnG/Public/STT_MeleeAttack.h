// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "CoreMinimal.h"
#include "STT_MeleeAttack.generated.h"

/**
 * 
 */
UCLASS()
class GNG_API USTT_MeleeAttack : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()
	
protected:
	
	
	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition
	) override;

	virtual void ExitState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition
	) override;
	
	UFUNCTION()
	void HitCheck();
	
	UPROPERTY(Blueprintable)
	FVector AttackCenter;
	
	FTimerHandle DelayTimerHandle;
	
public:
	
	// The Character
	UPROPERTY(VisibleAnywhere, Category="Context")
	ACharacter* Character;
	
	// Variables that should be able to be changed in the state tree
	UPROPERTY(EditAnywhere)
	float WindUP; // Delay between the start of the animation to the attack
	
	UPROPERTY(EditAnywhere)
	float AttackRange = 100; // Range of the Attack
	
	UPROPERTY(EditAnywhere)
	float AttackDamage = 10; 
	
	UPROPERTY(EditAnywhere)
	USoundBase* AttackSound; // Sound played at the start of the attack
	
	UPROPERTY(EditAnywhere)
	UAnimationAsset* AttackAnim; // Anim played at the start of the Attack
};
