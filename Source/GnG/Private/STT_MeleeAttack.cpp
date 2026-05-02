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
	Super::EnterState(Context, Transition);

	if (!Character)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	UGameplayStatics::PlaySoundAtLocation(
		this,
		AttackSound,
		Character->GetActorLocation());
	
	Character->GetMesh()->PlayAnimation(AttackAnim, false);
	
	GetWorld()->GetTimerManager().SetTimer(
		DelayTimerHandle,
		this,
		&USTT_MeleeAttack::HitCheck,
		WindUP,
		false);

	return EStateTreeRunStatus::Running;
}

void USTT_MeleeAttack::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DelayTimerHandle);
	}

	Super::ExitState(Context, Transition);
}

void USTT_MeleeAttack::HitCheck()
{
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!Character || !PlayerCharacter)
	{
		FinishTask(false);
		return;
	}

	FVector CharacterLocation = Character->GetActorLocation();
	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	FVector Direction = (PlayerLocation - CharacterLocation).GetSafeNormal();
	AttackCenter = CharacterLocation + Direction * AttackRange;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
	TArray<AActor*> OutActors;

	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		AttackCenter,
		AttackRange,
		ObjectTypes,
		APawn::StaticClass(),
		TArray<AActor*>(),
		OutActors
	);

	for (AActor* OutActor : OutActors)
	{
		if (OutActor == PlayerCharacter)
		{
			// Applies damage
			UGameplayStatics::ApplyDamage(
				PlayerCharacter,
				AttackDamage,
				nullptr,
				Character,
				UDamageType::StaticClass()
			);
		}
	}

	FinishTask(true);
}
