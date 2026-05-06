// Main player character class for the third-person version of the game.

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
	// Constructor that sets the player's default values and components.
	AJarl_ThirdPersonCharacter_CPP();

protected:
	// Runs when the character enters the level.
	virtual void BeginPlay() override;
	
	// Input handlers for movement and camera control.
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
	// Input handlers for movement abilities and combat actions.
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
	
	// Input references from the Enhanced Input system.
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
	
	// Camera used for aiming and first-person style view control.
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category="Input")
	UCameraComponent* FollowCamera;
	
	// Tracks whether movement input is currently being pressed.
	UPROPERTY(EditAnywhere, Category="Movement")
	bool bIsMoving;

	// Slide tuning values.
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
	
	// Tracks how long the camera has stayed underwater.
	UPROPERTY(EditAnywhere, Category="Water")
	float UnderwaterTimer;
	
	// Time between each drowning damage tick.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Water")
	float DrownSpeed = 1.0f;

	// Health values.
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

	// Sounds for taking damage and dying.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio")
	USoundBase* HitSoundA = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio")
	USoundBase* HitSoundB = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio")
	USoundBase* DeathSound = nullptr;

	// Projectile spawner setup.
	UPROPERTY(EditAnywhere, Category = "Weapons")
	TSubclassOf<AProjectileSpawner> SpawnerClass;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapons")
	AProjectileSpawner* Spawner;

	UPROPERTY(EditAnywhere, Category = "Weapons")
	FName SpawnerAttachSocket = NAME_None;

	// Default weapons for the three hotbar slots.
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

	// Water state values for the camera.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water")
	bool bIsCameraUnderRiver;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water")
	float CameraDepthUnderRiver;
	
	UPROPERTY()
	AEnemyParent* EnemyParentInstance;
	
	// Default movement values saved so they can be restored after sliding.
	float DefaultGroundFriction;
	float DefaultWalkSpeed;
	float DefaultBraking;
	float DefaultCrouchedWalkSpeed;

	// Runtime slide state.
	bool bIsSliding;
	bool bIsSprinting;
	bool bCanSlide;
	FTimerHandle SlideTimerHandle;
	FTimerHandle SlideCooldownTimerHandle;
	FVector SlideDirection;
	float SlideInitialSpeed;
	float SlideTargetEndSpeed;
	float SlideElapsedTime;

	// Camera offset used during the death animation.
	float CameraHeight;

	// Helper functions for water checks, HUD refreshes, and damage state.
	bool UpdateCameraRiverOverlap();
	void UpdateSlide(float DeltaTime);
	void ResetSlideCooldown();
	void UpdateHealthHUD() const;
	void UpdateShieldHUD() const;
	void RefreshVitalHUD() const;
	void HandlePlayerDeath();
	void ClearPostHitInvulnerability();
	
	
	
public:	
	// Runs every frame.
	virtual void Tick(float DeltaTime) override;

	// Connects actions and keys to their matching functions.
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// Updates the score and handles the win condition.
	UFUNCTION(BlueprintCallable)
	void UpdateScore(float Amount, bool bIsCyclops);

	// Adds shields up to the maximum amount.
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddShields(int32 ShieldAmount);

	// Handles incoming damage, shield loss, and death.
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// Hotbar helpers.
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
	
	// Stores the current run time for the end screen.
	UPROPERTY(BlueprintReadWrite, Category="Stats")
	float GameTimer;
	
	UPROPERTY(EditAnywhere)
	bool bTakesDamage; // Debug flag for damage-related flow.

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats")
	bool bIsPostHitInvulnerable;
	
	// Timers for win state, invulnerability, and death handling.
	FTimerHandle WinTimerHandle; // Delays the win screen.
	FTimerHandle PostHitInvulnerabilityTimerHandle;
	FTimerHandle DeathTimerHandle;
	
	UFUNCTION()
	void ChangeSceene(); // Opens the victory screen after the timer ends.
};
