// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "THCharater.generated.h"

class AProjectileSpawner;
class AProjectile_Base;
class AWeaponselector;
class UCameraComponent;
class USpringArmComponent;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;

UCLASS()
class GNG_API ATHCharater : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATHCharater();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/*
	 * Controls
	 */
	UPROPERTY(EditAnywhere)
	UInputMappingContext* MappingContext;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ShootAction;
	
	// Fires the currently selected weapon through the shared projectile spawner.
	void PlayerShoot();

	// Forwards the jump input to the built-in Character movement behavior.
	void PlayerJump();

	// Converts 2D movement input into world-space forward/right movement.
	void PlayerMove(const FInputActionValue& ActionValue);

	// Applies mouse/controller look input to the player controller.
	void PlayerLook(const FInputActionValue& ActionValue);

	// Convenience bindings for the hotbar keys.
	void SelectPrimaryWeapon();
	void SelectSecondaryWeapon();
	void SelectTertiaryWeapon();
	
	/*
	 * Camera
	 */
	
	UPROPERTY(EditAnywhere, Category = "Camera")
	UCameraComponent* PlayerCamera;
	
	UPROPERTY(EditAnywhere, Category = "camera")
	USpringArmComponent* SpringArm;
	
	/*
	 * Weapons
	 */
	
	UPROPERTY(EditAnywhere, Category = "Weapons")
	TSubclassOf<AProjectileSpawner> SpawnerClass;

	// Runtime reference to the spawned spawner actor used by the fire input.
	UPROPERTY(VisibleInstanceOnly, Category = "Weapons")
	AProjectileSpawner* Spawner;

	// Optional socket name on the skeletal mesh where the spawner should attach.
	UPROPERTY(EditAnywhere, Category = "Weapons")
	FName SpawnerAttachSocket = NAME_None;

	// Optional second starting weapon for slot 2.
	// Optional second weapon to pre-populate slot 2 at BeginPlay.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hotbar")
	TSubclassOf<AProjectile_Base> DefaultSecondaryWeaponClass;

	// TEMP TESTING: remove this property and key 3 binding when reverting the hotbar back to 2 slots.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hotbar")
	TSubclassOf<AProjectile_Base> DefaultTertiaryWeaponClass;

	// Lets you swap in a Blueprint subclass of the selector, but the native class works by default.
	UPROPERTY(EditAnywhere, Category = "Hotbar")
	TSubclassOf<AWeaponselector> WeaponSelectorClass;

	// Runtime hotbar manager that owns slot state and pushes weapon changes to the spawner.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Hotbar")
	AWeaponselector* WeaponSelector;

	// Pickup entry point used by Blueprints to add or replace weapons in the character hotbar.
	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	bool AddWeaponToHotbar(TSubclassOf<AProjectile_Base> WeaponClass);

	// Selects a hotbar slot by index and updates the spawner's projectile class.
	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	bool SelectWeaponSlot(int32 SlotIndex);

	// Returns the weapon class currently stored in the requested hotbar slot.
	UFUNCTION(BlueprintPure, Category = "Hotbar")
	TSubclassOf<AProjectile_Base> GetWeaponInSlot(int32 SlotIndex) const;

	// Returns the slot index currently feeding the projectile spawner.
	UFUNCTION(BlueprintPure, Category = "Hotbar")
	int32 GetActiveWeaponSlot() const;

};
