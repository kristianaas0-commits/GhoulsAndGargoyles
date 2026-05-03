// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Jarl_ThirdPersonCharacter_CPP.generated.h"

class AEnemyParent;
class AProjectileSpawner;
class AProjectile_Base;
class AWeaponselector;
class AWaterBodyRiver;
class AController;
class AActor;
class UCameraComponent;
class USoundBase;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;

UCLASS()
class GNG_API AJarl_ThirdPersonCharacter_CPP : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AJarl_ThirdPersonCharacter_CPP();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Input functions
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
	void StartJump();
	void StopJump();
	void Slide();
	void Sprint();
	void StopSprint();
	void StopSlide();
	void PlayerShoot();
	void TogglePause();
	void SelectPrimaryWeapon();
	void SelectSecondaryWeapon();
	void SelectThirdWeapon();
	
	// Controller
	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* MappingContext;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SlideAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ShootAction;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category="Input")
	UCameraComponent* FollowCamera;
	
	// Variables
	UPROPERTY(EditAnywhere, Category="Movement")
	bool bIsMoving;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Slide")
	float SlideDuration = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Slide")
	float SlideMinStartSpeedFraction = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Slide")
	float SlideStartSpeedMultiplier = 1.18f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Slide")
	float SlideMaxSprintMultiplier = 1.55f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Slide")
	float SlideStartSpeedBonus = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Slide")
	float SlideEndSpeedMultiplier = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Slide")
	float SlideGroundFriction = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Slide")
	float SlideBrakingDeceleration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement|Slide")
	float SlideCooldown = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float Score;
	
	UPROPERTY(EditAnywhere, Category="Water")
	float UnderwaterTimer;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Water")
	float DrownSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
	int32 MaxHits;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats")
	int32 HitsRemaining;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
	int32 MaxShields;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats")
	int32 ShieldsRemaining;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
	float PostHitInvulnerabilityDuration = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio")
	USoundBase* HitSoundA = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio")
	USoundBase* HitSoundB = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio")
	USoundBase* DeathSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "Weapons")
	TSubclassOf<AProjectileSpawner> SpawnerClass;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapons")
	AProjectileSpawner* Spawner;

	UPROPERTY(EditAnywhere, Category = "Weapons")
	FName SpawnerAttachSocket = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hotbar")
	TSubclassOf<AProjectile_Base> DefaultPrimaryWeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hotbar")
	TSubclassOf<AProjectile_Base> DefaultSecondaryWeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hotbar")
	TSubclassOf<AProjectile_Base> DefaultTertiaryWeaponClass;

	UPROPERTY(EditAnywhere, Category = "Hotbar")
	TSubclassOf<AWeaponselector> WeaponSelectorClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Hotbar")
	AWeaponselector* WeaponSelector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water")
	bool bIsCameraUnderRiver;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water")
	float CameraDepthUnderRiver;
	
	UPROPERTY()
	AEnemyParent* EnemyParentInstance;
	
	// Movement settings
	float DefaultGroundFriction;
	float DefaultWalkSpeed;
	float DefaultBraking;
	float DefaultCrouchedWalkSpeed;
	bool bIsSliding;
	bool bIsSprinting;
	bool bCanSlide;
	FTimerHandle SlideTimerHandle;
	FTimerHandle SlideCooldownTimerHandle;
	FVector SlideDirection;
	float SlideInitialSpeed;
	float SlideTargetEndSpeed;
	float SlideElapsedTime;
	float CameraHeight;

	bool UpdateCameraRiverOverlap();
	void UpdateSlide(float DeltaTime);
	void ResetSlideCooldown();
	void UpdateHealthHUD() const;
	void UpdateShieldHUD() const;
	void RefreshVitalHUD() const;
	void HandlePlayerDeath();
	void ClearPostHitInvulnerability();
	
	
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// Score function
	UFUNCTION(BlueprintCallable)
	void UpdateScore(float Amount, bool bIsCyclops);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddShields(int32 ShieldAmount);

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	bool SelectWeaponSlot(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Hotbar")
	TSubclassOf<AProjectile_Base> GetWeaponInSlot(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Hotbar")
	int32 GetActiveWeaponSlot() const;

	UFUNCTION(BlueprintPure, Category = "Water")
	bool IsCameraUnderRiver() const { return bIsCameraUnderRiver; }
	
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetHitsRemaining() const { return HitsRemaining; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetShieldsRemaining() const { return ShieldsRemaining; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	bool IsDead() const { return HitsRemaining <= 0; }
	
	UPROPERTY(BlueprintReadWrite, Category="Stats")
	float GameTimer;
	
	UPROPERTY(EditAnywhere)
	bool bTakesDamage; // Debug

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats")
	bool bIsPostHitInvulnerable;
	
	/*
	 * Victory
	 */
	FTimerHandle WinTimerHandle; // Timer handle to delay the victory screen
	FTimerHandle PostHitInvulnerabilityTimerHandle;
	FTimerHandle DeathTimerHandle;
	
	UFUNCTION()
	void ChangeSceene(); // Function to change to the victory screen after a delay
};
