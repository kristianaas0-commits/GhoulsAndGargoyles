// Fill out your copyright notice in the Description page of Project Settings.


#include "Jarl_ThirdPersonCharacter_CPP.h"

#include "EnemyParent.h"
#include "EngineUtils.h"
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
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerInput.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "InputCoreTypes.h"
#include "MyGameInstance.h"
#include "Camera/CameraComponent.h"
#include "Blueprint/UserWidget.h"
#include "UObject/UnrealType.h"
#include "WaterBodyComponent.h"
#include "WaterBodyRiverActor.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

class UEnhancedInputLocalPlayerSubsystem;
// Sets default values
AJarl_ThirdPersonCharacter_CPP::AJarl_ThirdPersonCharacter_CPP()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bIsMoving = false;
	Score = 0;
	MaxHits = 3;
	HitsRemaining = MaxHits;
	Spawner = nullptr;
	WeaponSelector = nullptr;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(GetRootComponent());
	FollowCamera->bUsePawnControlRotation = true;
	FollowCamera->AddWorldOffset(FVector(0.f,0.f,70.f));
	
	// Prefer the Blueprint child so slot 1 uses the configured Lance asset instead of the raw C++ parent.
	static ConstructorHelpers::FClassFinder<AProjectile_Base> LanceBlueprintClass(TEXT("/Game/Weapons/Projectiles/Lance"));
	if (LanceBlueprintClass.Succeeded())
	{
		DefaultPrimaryWeaponClass = LanceBlueprintClass.Class;
	}
	else
	{
		// Fall back to the C++ class if the Blueprint asset path changes or cannot be found.
		DefaultPrimaryWeaponClass = ALanceCPP::StaticClass();
	}

	DefaultSecondaryWeaponClass = ATorchCPP::StaticClass();
	DefaultTertiaryWeaponClass = AHeavyAxe::StaticClass();
	bIsCameraUnderRiver = false;
	CameraDepthUnderRiver = 0.0f;
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
	
	HitsRemaining = FMath::Clamp(MaxHits, 0, MaxHits);
	UpdateHealthHUD();

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
		WeaponSelector->InitializeWeaponSelector(
			Spawner,
			DefaultPrimaryWeaponClass,
			DefaultSecondaryWeaponClass,
			DefaultTertiaryWeaponClass);
	}
	
	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyParent::StaticClass(), FoundEnemies);

	for (AActor* FoundActor : FoundEnemies)
	{
		if (AEnemyParent* Enemy = Cast<AEnemyParent>(FoundActor))
		{
			Enemy->OnDeathForScore.AddDynamic(this, &AJarl_ThirdPersonCharacter_CPP::UpdateScore);
		}
	}
}

// Called every frame
void AJarl_ThirdPersonCharacter_CPP::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateCameraRiverOverlap();
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

	FInputKeyBinding& PauseBindingEscape = PlayerInputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AJarl_ThirdPersonCharacter_CPP::TogglePause);
	PauseBindingEscape.bExecuteWhenPaused = true;

	FInputKeyBinding& PauseBindingP = PlayerInputComponent->BindKey(EKeys::P, IE_Pressed, this, &AJarl_ThirdPersonCharacter_CPP::TogglePause);
	PauseBindingP.bExecuteWhenPaused = true;

	PlayerInputComponent->BindKey(EKeys::One, IE_Pressed, this, &AJarl_ThirdPersonCharacter_CPP::SelectPrimaryWeapon);
	PlayerInputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AJarl_ThirdPersonCharacter_CPP::SelectSecondaryWeapon);
	PlayerInputComponent->BindKey(EKeys::Three, IE_Pressed, this, &AJarl_ThirdPersonCharacter_CPP::SelectThirdWeapon);
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
			const FVector SpawnLocation = FollowCamera->GetComponentLocation() + (FollowCamera->GetForwardVector() * 100.0f) + (FollowCamera->GetRightVector() * 30.f + FVector(0.f,0.f,-20.f));
			const FRotator SpawnRotation = Controller ? Controller->GetControlRotation() : FollowCamera->GetComponentRotation();
			Spawner->Fire(SpawnLocation, SpawnRotation);
		
			if (Spawner && Spawner->ProjectileActor)
			{
				if (const AProjectile_Base* ProjectileSounds = Spawner->ProjectileActor->GetDefaultObject<AProjectile_Base>())
				{
					UGameplayStatics::PlaySoundAtLocation(this, ProjectileSounds->ThrowSound, GetActorLocation());
				}
			}
		}
}

void AJarl_ThirdPersonCharacter_CPP::TogglePause()
{
	if (!GetWorld())
	{
		return;
	}

	const bool bIsPaused = UGameplayStatics::IsGamePaused(this);
	UGameplayStatics::SetGamePaused(this, !bIsPaused);
}

void AJarl_ThirdPersonCharacter_CPP::SelectPrimaryWeapon()
{
	SelectWeaponSlot(0);
}

void AJarl_ThirdPersonCharacter_CPP::SelectSecondaryWeapon()
{
	SelectWeaponSlot(1);
}

void AJarl_ThirdPersonCharacter_CPP::SelectThirdWeapon()
{
	SelectWeaponSlot(2);
}

void AJarl_ThirdPersonCharacter_CPP::UpdateScore(float Amount, bool bIsCyclops)
{
	Score += Amount;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			reinterpret_cast<uint64>(this) + 3,
			2.0f,
			FColor::Yellow,
			FString::Printf(TEXT("Amount: %.2f | Score: %.2f"), Amount, Score));
	}

	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		AHUD* HUD = PlayerController->GetHUD();
		if (HUD)
		{
			const FObjectProperty* ScoreWidgetProperty = FindFProperty<FObjectProperty>(HUD->GetClass(), TEXT("UI_ScoreRef"));
			if (ScoreWidgetProperty)
			{
				UObject* ScoreWidgetObject = ScoreWidgetProperty->GetObjectPropertyValue_InContainer(HUD);
				UUserWidget* ScoreWidget = Cast<UUserWidget>(ScoreWidgetObject);
				if (ScoreWidget)
				{
					UFunction* ChangeScoreFunction = ScoreWidget->FindFunction(TEXT("ChangeScore"));
					if (ChangeScoreFunction)
					{
						const float CurrentScoreValue = Score;
						TArray<uint8> ParamBuffer;
						ParamBuffer.SetNumZeroed(ChangeScoreFunction->ParmsSize);

						if (FProperty* ScoreParamProperty = ChangeScoreFunction->FindPropertyByName(TEXT("Score")))
						{
							if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(ScoreParamProperty))
							{
								FloatProperty->SetFloatingPointPropertyValue(ParamBuffer.GetData(), CurrentScoreValue);
							}
							else if (FDoubleProperty* DoubleProperty = CastField<FDoubleProperty>(ScoreParamProperty))
							{
								DoubleProperty->SetFloatingPointPropertyValue(ParamBuffer.GetData(), static_cast<double>(CurrentScoreValue));
							}
						}

						ScoreWidget->ProcessEvent(ChangeScoreFunction, ParamBuffer.GetData());
					}
				}
			}
		}
	}

	if (bIsCyclops)
	{
		UMyGameInstance*GI = Cast<UMyGameInstance>(GetGameInstance());
		if (GI)
		{
			GI->EndGameTimer = GameTimer;
			GI->EndScore = Score;
		}
		
		GetWorldTimerManager().SetTimer(
		WinTimerHandle,
		this,
		&AJarl_ThirdPersonCharacter_CPP::ChangeSceene,
		5.f,
		false)
	;
		
	}
}

float AJarl_ThirdPersonCharacter_CPP::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageAmount <= 0.0f || HitsRemaining <= 0)
	{
		return 0.0f;
	}

	HitsRemaining = FMath::Max(0, HitsRemaining - 1);
	UpdateHealthHUD();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			reinterpret_cast<uint64>(this) + 1,
			2.0f,
			FColor::Red,
			FString::Printf(TEXT("Hits Remaining: %d"), HitsRemaining));
	}

	if (HitsRemaining <= 0)
	{
		HandlePlayerDeath();
	}

	return 1.0f;
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

void AJarl_ThirdPersonCharacter_CPP::ChangeSceene()
{
	UGameplayStatics::OpenLevel(this, FName("EndScreen"));
}

bool AJarl_ThirdPersonCharacter_CPP::UpdateCameraRiverOverlap()
{
	CameraDepthUnderRiver = 0.0f;
	bIsCameraUnderRiver = false;

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return false;
	}

	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	for (TActorIterator<AWaterBodyRiver> RiverIt(GetWorld()); RiverIt; ++RiverIt)
	{
		AWaterBodyRiver* River = *RiverIt;
		if (!River)
		{
			continue;
		}

		UWaterBodyComponent* WaterBodyComponent = River->GetWaterBodyComponent();
		if (!WaterBodyComponent)
		{
			continue;
		}

		const TValueOrError<FWaterBodyQueryResult, EWaterBodyQueryError> QueryResult =
			WaterBodyComponent->TryQueryWaterInfoClosestToWorldLocation(
				CameraLocation,
				EWaterBodyQueryFlags::ComputeLocation | EWaterBodyQueryFlags::ComputeImmersionDepth);
		if (!QueryResult.HasValue())
		{
			continue;
		}

		const FWaterBodyQueryResult& WaterInfo = QueryResult.GetValue();
		if (WaterInfo.IsInWater() && !WaterInfo.IsInExclusionVolume())
		{
			bIsCameraUnderRiver = true;
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					reinterpret_cast<uint64>(this),
					0.0f,
					FColor::Cyan,
					FString::Printf(TEXT("Camera under water")));
			}
			return true;
		}
	}

	return false;
}

void AJarl_ThirdPersonCharacter_CPP::UpdateHealthHUD() const
{
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	AHUD* HUD = PlayerController->GetHUD();
	if (!HUD)
	{
		return;
	}

	const FObjectProperty* HealthWidgetProperty = FindFProperty<FObjectProperty>(HUD->GetClass(), TEXT("UI_HealthRef"));
	if (!HealthWidgetProperty)
	{
		return;
	}

	UObject* HealthWidgetObject = HealthWidgetProperty->GetObjectPropertyValue_InContainer(HUD);
	UUserWidget* HealthWidget = Cast<UUserWidget>(HealthWidgetObject);
	if (!HealthWidget)
	{
		return;
	}

	UFunction* RefreshHeartsFunction = HealthWidget->FindFunction(TEXT("RefreshHearts"));
	if (!RefreshHeartsFunction)
	{
		return;
	}

	HealthWidget->ProcessEvent(RefreshHeartsFunction, nullptr);
}

void AJarl_ThirdPersonCharacter_CPP::HandlePlayerDeath()
{
	if (GEngine)
	{
		
		UMyGameInstance*GI = Cast<UMyGameInstance>(GetGameInstance());
		if (GI)
		{
			GI->EndGameTimer = GameTimer;
		}
		if (bTakesDamage)
		{
			UGameplayStatics::OpenLevel(this, FName("DeathScreen"));
		}
		GEngine->AddOnScreenDebugMessage(
			reinterpret_cast<uint64>(this) + 2,
			3.0f,
			FColor::Red,
			TEXT("Player defeated"));
		
	}
}
