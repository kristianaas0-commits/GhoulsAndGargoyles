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
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "InputCoreTypes.h"
#include "MyGameInstance.h"
#include "Camera/CameraComponent.h"
#include "Blueprint/UserWidget.h"
#include "UObject/UnrealType.h"
#include "WaterBodyComponent.h"
#include "WaterBodyRiverActor.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

class UEnhancedInputLocalPlayerSubsystem;

namespace
{
	UUserWidget* GetPlayerHealthWidget(const AJarl_ThirdPersonCharacter_CPP* Character)
	{
		if (!Character)
		{
			return nullptr;
		}

		const APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
		if (!PlayerController)
		{
			return nullptr;
		}

		AHUD* HUD = PlayerController->GetHUD();
		if (!HUD)
		{
			return nullptr;
		}

		const FObjectProperty* HealthWidgetProperty = FindFProperty<FObjectProperty>(HUD->GetClass(), TEXT("UI_HealthRef"));
		if (!HealthWidgetProperty)
		{
			return nullptr;
		}

		UObject* HealthWidgetObject = HealthWidgetProperty->GetObjectPropertyValue_InContainer(HUD);
		return Cast<UUserWidget>(HealthWidgetObject);
	}
}

// Sets default values
AJarl_ThirdPersonCharacter_CPP::AJarl_ThirdPersonCharacter_CPP()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bIsMoving = false;
	Score = 0;
	MaxHits = 3;
	MaxShields = 3;
	HitsRemaining = MaxHits;
	ShieldsRemaining = 0;
	Spawner = nullptr;
	WeaponSelector = nullptr;
	UnderwaterTimer = 0.0f;
	bIsPostHitInvulnerable = false;

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
	DefaultCrouchedWalkSpeed = GetCharacterMovement()->MaxWalkSpeedCrouched;
	
	bIsSliding = false;
	bIsSprinting = false;
	bCanSlide = true;
	SlideDirection = FVector::ZeroVector;
	SlideInitialSpeed = 0.0f;
	SlideTargetEndSpeed = 0.0f;
	SlideElapsedTime = 0.0f;
	
	HitsRemaining = FMath::Clamp(HitsRemaining, 0, MaxHits);
	ShieldsRemaining = FMath::Clamp(ShieldsRemaining, 0, MaxShields);
	RefreshVitalHUD();

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
	
	if (IsCameraUnderRiver())
	{
		UnderwaterTimer += DeltaTime;
		
		if (UnderwaterTimer >= DrownSpeed)
		{
			FDamageEvent DamageEvent;
			TakeDamage(10.0f, DamageEvent, GetController(), this);

            UnderwaterTimer -= DrownSpeed;
		}
	}
	else
	{
		UnderwaterTimer = 0.0f;
	}

	UpdateSlide(DeltaTime);
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
	if (!bCanSlide) return;
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp->IsMovingOnGround()) return;
	if (!bIsMoving) return;

	const FVector HorizontalVelocity(GetVelocity().X, GetVelocity().Y, 0.0f);
	const float CurrentSpeed = HorizontalVelocity.Size();
	const float MinSlideStartSpeed = DefaultWalkSpeed * SlideMinStartSpeedFraction;
	const float SprintSpeed = DefaultWalkSpeed * 1.8f;

	if (CurrentSpeed < MinSlideStartSpeed) return;

	bIsSliding = true;
	bCanSlide = false;
	SlideElapsedTime = 0.0f;

	Crouch();

	SlideDirection = HorizontalVelocity.GetSafeNormal();
	const float BurstFromCurrentSpeed = (CurrentSpeed * SlideStartSpeedMultiplier) + SlideStartSpeedBonus;
	const float MaxSlideSpeed = SprintSpeed * SlideMaxSprintMultiplier;
	SlideInitialSpeed = FMath::Clamp(
		BurstFromCurrentSpeed,
		CurrentSpeed,
		MaxSlideSpeed);
	SlideTargetEndSpeed = FMath::Max(DefaultWalkSpeed * SlideEndSpeedMultiplier, DefaultCrouchedWalkSpeed);

	MoveComp->GroundFriction = SlideGroundFriction;
	MoveComp->BrakingDecelerationWalking = SlideBrakingDeceleration;
	MoveComp->MaxWalkSpeed = SlideInitialSpeed;
	MoveComp->MaxWalkSpeedCrouched = SlideInitialSpeed;
	MoveComp->Velocity = FVector(SlideDirection.X * SlideInitialSpeed, SlideDirection.Y * SlideInitialSpeed, MoveComp->Velocity.Z);

	GetWorldTimerManager().SetTimer(SlideTimerHandle, this, &AJarl_ThirdPersonCharacter_CPP::StopSlide, SlideDuration, false);
	GetWorldTimerManager().SetTimer(SlideCooldownTimerHandle, this, &AJarl_ThirdPersonCharacter_CPP::ResetSlideCooldown, SlideCooldown, false);
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
	MoveComp->MaxWalkSpeedCrouched = DefaultCrouchedWalkSpeed;
	
	MoveComp->GroundFriction = DefaultGroundFriction;
	MoveComp->BrakingDecelerationWalking = DefaultBraking;
	
	GetWorldTimerManager().ClearTimer(SlideTimerHandle);

	bIsSliding = false;
	SlideDirection = FVector::ZeroVector;
	SlideInitialSpeed = 0.0f;
	SlideTargetEndSpeed = 0.0f;
	SlideElapsedTime = 0.0f;
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

void AJarl_ThirdPersonCharacter_CPP::UpdateSlide(float DeltaTime)
{
	if (!bIsSliding)
	{
		return;
	}

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp || !MoveComp->IsMovingOnGround())
	{
		StopSlide();
		return;
	}

	SlideElapsedTime += DeltaTime;
	const float SlideAlpha = SlideDuration > 0.0f ? FMath::Clamp(SlideElapsedTime / SlideDuration, 0.0f, 1.0f) : 1.0f;
	const float CurrentSlideSpeed = FMath::InterpEaseOut(SlideInitialSpeed, SlideTargetEndSpeed, SlideAlpha, 4.0f);
	const FVector CurrentHorizontalVelocity(MoveComp->Velocity.X, MoveComp->Velocity.Y, 0.0f);
	if (!CurrentHorizontalVelocity.IsNearlyZero())
	{
		SlideDirection = CurrentHorizontalVelocity.GetSafeNormal();
	}

	MoveComp->MaxWalkSpeed = CurrentSlideSpeed;
	MoveComp->MaxWalkSpeedCrouched = CurrentSlideSpeed;
	MoveComp->Velocity = FVector(SlideDirection.X * CurrentSlideSpeed, SlideDirection.Y * CurrentSlideSpeed, MoveComp->Velocity.Z);
}

void AJarl_ThirdPersonCharacter_CPP::ResetSlideCooldown()
{
	bCanSlide = true;
}

void AJarl_ThirdPersonCharacter_CPP::PlayerShoot()
{
	if (!Spawner)
	{
		return;
	}
			const FVector CameraLocation = FollowCamera->GetComponentLocation();
			const FVector CameraRotation = FollowCamera->GetForwardVector();

			const FVector SpawnLocation = FollowCamera->GetComponentLocation() + (FollowCamera->GetForwardVector() * 100.0f) + (FollowCamera->GetRightVector() * 30.f + FVector(0.f,0.f,-20.f));
			const FVector TraceEnd = CameraLocation + (CameraRotation * 10000.0f);
	
			FHitResult HitResult;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(this);
			QueryParams.AddIgnoredActor(Spawner);
	
			const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult,CameraLocation,TraceEnd,ECC_Visibility,QueryParams);
			
			const FVector AimPoint = bHit ? HitResult.Location : TraceEnd;
			const FRotator AimRotation = (AimPoint - SpawnLocation).Rotation();
	
			//plays a sound when its actually Fired
			if (AProjectile_Base* Projectile = Spawner->Fire(SpawnLocation, AimRotation))
			{
				if (Projectile->ThrowSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, Projectile->ThrowSound, GetActorLocation());
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
		4.f,
		false)
	;
		
	}
}

void AJarl_ThirdPersonCharacter_CPP::AddShields(int32 ShieldAmount)
{
	if (ShieldAmount <= 0)
	{
		return;
	}

	ShieldsRemaining = FMath::Clamp(ShieldsRemaining + ShieldAmount, 0, MaxShields);
	UpdateShieldHUD();
}

float AJarl_ThirdPersonCharacter_CPP::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageAmount <= 0.0f || HitsRemaining <= 0 || bIsPostHitInvulnerable)
	{
		return 0.0f;
	}

	auto PlayHitSound = [this]()
	{
		TArray<USoundBase*, TInlineAllocator<2>> AvailableHitSounds;
		if (HitSoundA)
		{
			AvailableHitSounds.Add(HitSoundA);
		}
		if (HitSoundB)
		{
			AvailableHitSounds.Add(HitSoundB);
		}

		if (AvailableHitSounds.Num() > 0)
		{
			const int32 SelectedIndex = FMath::RandRange(0, AvailableHitSounds.Num() - 1);
			UGameplayStatics::PlaySoundAtLocation(this, AvailableHitSounds[SelectedIndex], GetActorLocation());
		}
	};

	auto StartPostHitInvulnerability = [this]()
	{
		if (PostHitInvulnerabilityDuration <= 0.0f)
		{
			return;
		}

		bIsPostHitInvulnerable = true;
		GetWorldTimerManager().SetTimer(
			PostHitInvulnerabilityTimerHandle,
			this,
			&AJarl_ThirdPersonCharacter_CPP::ClearPostHitInvulnerability,
			PostHitInvulnerabilityDuration,
			false);
	};

	if (ShieldsRemaining > 0)
	{
		ShieldsRemaining = FMath::Max(0, ShieldsRemaining - 1);
		UpdateShieldHUD();
		PlayHitSound();
		StartPostHitInvulnerability();

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				reinterpret_cast<uint64>(this) + 4,
				2.0f,
				FColor::Cyan,
				FString::Printf(TEXT("Shields Remaining: %d"), ShieldsRemaining));
		}

		return 1.0f;
	}

	HitsRemaining = FMath::Max(0, HitsRemaining - 1);
	UpdateHealthHUD();
	PlayHitSound();
	StartPostHitInvulnerability();

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

void AJarl_ThirdPersonCharacter_CPP::ClearPostHitInvulnerability()
{
	bIsPostHitInvulnerable = false;
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

void AJarl_ThirdPersonCharacter_CPP::RefreshVitalHUD() const
{
	UpdateHealthHUD();
	UpdateShieldHUD();
}

void AJarl_ThirdPersonCharacter_CPP::UpdateHealthHUD() const
{
	UUserWidget* HealthWidget = GetPlayerHealthWidget(this);
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

void AJarl_ThirdPersonCharacter_CPP::UpdateShieldHUD() const
{
	UUserWidget* HealthWidget = GetPlayerHealthWidget(this);
	if (!HealthWidget)
	{
		return;
	}

	UFunction* RefreshShieldsFunction = HealthWidget->FindFunction(TEXT("RefreshShields"));
	if (!RefreshShieldsFunction)
	{
		return;
	}

	HealthWidget->ProcessEvent(RefreshShieldsFunction, nullptr);
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
