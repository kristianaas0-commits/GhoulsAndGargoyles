// Fill out your copyright notice in the Description page of Project Settings.


#include "Jarl_ThirdPersonCharacter_CPP.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HeavyAxe.h"
#include "InputMappingContext.h"
#include "LanceCPP.h"
#include "ProjectileSpawner.h"
#include "Projectile_Base.h"
#include "TorchCPP.h"
#include "Weaponselector.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerInput.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"

class UEnhancedInputLocalPlayerSubsystem;
// Sets default values
AJarl_ThirdPersonCharacter_CPP::AJarl_ThirdPersonCharacter_CPP()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bIsMoving = false;
	Score = 0;
	Health = 100.0f;
	MaxHealth = 100.0f;
	Spawner = nullptr;
	WeaponSelector = nullptr;
	DefaultSecondaryWeaponClass = ATorchCPP::StaticClass();
	DefaultTertiaryWeaponClass = AHeavyAxe::StaticClass();
}

// Called when the game starts or when spawned
void AJarl_ThirdPersonCharacter_CPP::BeginPlay()
{
	Super::BeginPlay();
	
	// Hide the full body mesh for the owning player in first-person view.
	GetMesh()->SetOwnerNoSee(true);
	
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
			{
				Subsystem->AddMappingContext(MappingContext, 0);
			}
		}
	}
	
	// Movement settings
	DefaultGroundFriction = GetCharacterMovement()->GroundFriction;
	DefaultWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	DefaultBraking = GetCharacterMovement()->BrakingDecelerationWalking;
	
	bIsSliding = false;
	bIsSprinting = false;
	
	Health = MaxHealth;

	if (SpawnerClass && !Spawner)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.Instigator = this;
		Spawner = GetWorld()->SpawnActor<AProjectileSpawner>(SpawnerClass, GetActorLocation(), GetActorRotation(), SpawnParameters);
		if (Spawner)
		{
			Spawner->SetActorEnableCollision(false);
			if (SpawnerAttachSocket != NAME_None)
			{
				Spawner->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SpawnerAttachSocket);
			}
			else
			{
				Spawner->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
				Spawner->SetActorRelativeLocation(FVector::ZeroVector);
				Spawner->SetActorRelativeRotation(FRotator::ZeroRotator);
			}
		}
	}

	if (!WeaponSelector)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.Instigator = this;
		UClass* SelectorClassToSpawn = WeaponSelectorClass ? WeaponSelectorClass.Get() : AWeaponselector::StaticClass();
		WeaponSelector = GetWorld()->SpawnActor<AWeaponselector>(SelectorClassToSpawn, GetActorLocation(), GetActorRotation(), SpawnParameters);
	}

	if (WeaponSelector)
	{
		TSubclassOf<AProjectile_Base> DefaultPrimaryWeaponClass = ALanceCPP::StaticClass();
		if (Spawner && Spawner->ProjectileActor)
		{
			DefaultPrimaryWeaponClass = Spawner->ProjectileActor;
		}
		WeaponSelector->InitializeWeaponSelector(Spawner, DefaultPrimaryWeaponClass, DefaultSecondaryWeaponClass);
		WeaponSelector->AddWeaponToHotbar(DefaultTertiaryWeaponClass);
	}
}

// Called every frame
void AJarl_ThirdPersonCharacter_CPP::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AJarl_ThirdPersonCharacter_CPP::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AJarl_ThirdPersonCharacter_CPP::StartJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AJarl_ThirdPersonCharacter_CPP::StopJump);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AJarl_ThirdPersonCharacter_CPP::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AJarl_ThirdPersonCharacter_CPP::Look);
		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Started, this, &AJarl_ThirdPersonCharacter_CPP::Slide);
		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Completed, this, &AJarl_ThirdPersonCharacter_CPP::StopSlide);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AJarl_ThirdPersonCharacter_CPP::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AJarl_ThirdPersonCharacter_CPP::StopSprint);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &AJarl_ThirdPersonCharacter_CPP::PlayerShoot);
	}

	PlayerInputComponent->BindKey(EKeys::One, IE_Pressed, this, &AJarl_ThirdPersonCharacter_CPP::SelectPrimaryWeapon);
	PlayerInputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AJarl_ThirdPersonCharacter_CPP::SelectSecondaryWeapon);
	PlayerInputComponent->BindKey(EKeys::Three, IE_Pressed, this, &AJarl_ThirdPersonCharacter_CPP::SelectTertiaryWeapon);
}

void AJarl_ThirdPersonCharacter_CPP::Move(const FInputActionValue& Value)
{
	FVector2D ActionVector = Value.Get<FVector2D>();
	
	bIsMoving = !ActionVector.IsNearlyZero();

	// Ignore pitch when moving so forward always stays parallel to the ground.
	const FRotator ControlRotation = Controller ? Controller->GetControlRotation() : GetActorRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X),ActionVector.Y);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y),ActionVector.X);
}

void AJarl_ThirdPersonCharacter_CPP::Look(const FInputActionValue& Value)
{
	FVector2D InputVector = Value.Get<FVector2D>();
	
	AddControllerYawInput(InputVector.X);
	AddControllerPitchInput(InputVector.Y);
}

void AJarl_ThirdPersonCharacter_CPP::StartJump()
{
	Jump();
}

void AJarl_ThirdPersonCharacter_CPP::StopJump()
{
	StopJumping();
}

void AJarl_ThirdPersonCharacter_CPP::Slide()
{
	if (bIsSliding) return;
	if (!GetCharacterMovement()->IsMovingOnGround()) return;
	if (!bIsMoving) return;
	
	bIsSliding = true;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();

	Crouch();

	FVector VelocityDir = GetVelocity().GetSafeNormal();

	LaunchCharacter(VelocityDir * 1200.0f, true, true);

	MoveComp->GroundFriction = 0.0f;
	MoveComp->BrakingDecelerationWalking = 0.0f;
	MoveComp->MaxWalkSpeed = DefaultWalkSpeed * 3.0f;

	GetWorldTimerManager().SetTimer(SlideTimerHandle, this, &AJarl_ThirdPersonCharacter_CPP::StopSlide, 1.0f, false);
}

void AJarl_ThirdPersonCharacter_CPP::StopSlide()
{
	if (!bIsSliding) return;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();

	UnCrouch();

	if (bIsSprinting)
	{
		MoveComp->MaxWalkSpeed = DefaultWalkSpeed * 2;
	}
	else
	{
		MoveComp->MaxWalkSpeed = DefaultWalkSpeed;
	}
	
	MoveComp->GroundFriction = DefaultGroundFriction;
	MoveComp->BrakingDecelerationWalking = DefaultBraking;
	
	GetWorldTimerManager().ClearTimer(SlideTimerHandle);

	bIsSliding = false;
}

void AJarl_ThirdPersonCharacter_CPP::Sprint()
{
	bIsSprinting = true;

	if (!bIsSliding)
	{
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed * 2;
	}
}

void AJarl_ThirdPersonCharacter_CPP::StopSprint()
{
	bIsSprinting = false;

	if (!bIsSliding)
	{
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	}
}

void AJarl_ThirdPersonCharacter_CPP::PlayerShoot()
{
	if (Spawner)
	{
		const FVector SpawnLocation = GetActorLocation() + (GetActorForwardVector() * 100.f) + FVector(0.f, 0.f, 50.f);
		const FRotator SpawnRotation = Controller ? Controller->GetControlRotation() : GetActorRotation();
		Spawner->Fire(SpawnLocation, SpawnRotation);
	}
}

void AJarl_ThirdPersonCharacter_CPP::SelectPrimaryWeapon()
{
	SelectWeaponSlot(0);
}

void AJarl_ThirdPersonCharacter_CPP::SelectSecondaryWeapon()
{
	SelectWeaponSlot(1);
}

void AJarl_ThirdPersonCharacter_CPP::SelectTertiaryWeapon()
{
	SelectWeaponSlot(2);
}

void AJarl_ThirdPersonCharacter_CPP::UpdateScore(int32 Amount)
{
	Score += Amount;
}

bool AJarl_ThirdPersonCharacter_CPP::AddWeaponToHotbar(TSubclassOf<AProjectile_Base> WeaponClass)
{
	return WeaponSelector ? WeaponSelector->AddWeaponToHotbar(WeaponClass) : false;
}

bool AJarl_ThirdPersonCharacter_CPP::SelectWeaponSlot(int32 SlotIndex)
{
	return WeaponSelector ? WeaponSelector->SelectWeaponSlot(SlotIndex) : false;
}

TSubclassOf<AProjectile_Base> AJarl_ThirdPersonCharacter_CPP::GetWeaponInSlot(int32 SlotIndex) const
{
	return WeaponSelector ? WeaponSelector->GetWeaponInSlot(SlotIndex) : nullptr;
}

int32 AJarl_ThirdPersonCharacter_CPP::GetActiveWeaponSlot() const
{
	return WeaponSelector ? WeaponSelector->GetActiveWeaponSlot() : INDEX_NONE;
}
