// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Jarl_ThirdPersonCharacter_CPP.generated.h"

class AProjectileSpawner;
class AProjectile_Base;
class AWeaponselector;
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
	void SelectPrimaryWeapon();
	void SelectSecondaryWeapon();
	void SelectTertiaryWeapon();
	
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
	
	// Variables
	UPROPERTY(EditAnywhere, Category="Movement")
	bool bIsMoving;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	int32 Score;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float Health;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float MaxHealth;

	UPROPERTY(EditAnywhere, Category = "Weapons")
	TSubclassOf<AProjectileSpawner> SpawnerClass;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapons")
	AProjectileSpawner* Spawner;

	UPROPERTY(EditAnywhere, Category = "Weapons")
	FName SpawnerAttachSocket = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hotbar")
	TSubclassOf<AProjectile_Base> DefaultSecondaryWeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hotbar")
	TSubclassOf<AProjectile_Base> DefaultTertiaryWeaponClass;

	UPROPERTY(EditAnywhere, Category = "Hotbar")
	TSubclassOf<AWeaponselector> WeaponSelectorClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Hotbar")
	AWeaponselector* WeaponSelector;
	
	// Movement settings
	float DefaultGroundFriction;
	float DefaultWalkSpeed;
	float DefaultBraking;
	bool bIsSliding;
	bool bIsSprinting;
	FTimerHandle SlideTimerHandle;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// Score function
	UFUNCTION()
	void UpdateScore(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	bool AddWeaponToHotbar(TSubclassOf<AProjectile_Base> WeaponClass);

	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	bool SelectWeaponSlot(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Hotbar")
	TSubclassOf<AProjectile_Base> GetWeaponInSlot(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Hotbar")
	int32 GetActiveWeaponSlot() const;
};
