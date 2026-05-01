// Fill out your copyright notice in the Description page of Project Settings.


#include "STT_MeleeAttack.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"


EStateTreeRunStatus USTT_MeleeAttack::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	
	UGameplayStatics::PlaySoundAtLocation(
		this,
		AttackSound,
		Character->GetActorLocation()
	);

	Character->GetMesh()->PlayAnimation(AttackAnim, false);

	FVector CharacterLocation = Character->GetActorLocation();
	FVector PlayerLocation = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetActorLocation();
	FVector AttackRangeV = {AttackRange, AttackRange, 0};
	AttackCenter = CharacterLocation + (PlayerLocation - CharacterLocation).Normalize() * AttackRangeV;

	GetWorld()->GetTimerManager().SetTimer(
		DelayTimerHandle,
		this,
		&USTT_MeleeAttack::HitCheck,
		WindUP,
		false);
	
	FinishTask();
	
	//return EStateTreeRunStatus::Running;
	return EStateTreeRunStatus::Succeeded;
}

void USTT_MeleeAttack::HitCheck()
{
	TArray<AActor*> OutActors;

	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		AttackCenter,
		AttackRange,
		TArray<TEnumAsByte<EObjectTypeQuery>>(),
		APawn::StaticClass(),
		TArray<AActor*>(),
		OutActors
	);
	
	for (AActor* OutActor : OutActors)
	{
		if (OutActor == UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
		{
			// Applies damage
			UGameplayStatics::ApplyDamage(
				OutActor,
				AttackDamage,
				nullptr,
				Character,
				UDamageType::StaticClass()
				);
		}
	}
}

