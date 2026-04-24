// Fill out your copyright notice in the Description page of Project Settings.


#include "THCharater.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HeavyAxe.h"
#include "LanceCPP.h"
#include "ProjectileSpawner.h"
#include "Projectile_Base.h"
#include "TorchCPP.h"
#include "Weaponselector.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerInput.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"


// Sets default values
ATHCharater::ATHCharater()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = true;

	// Use a zero-length spring arm so the camera behaves like a first-person camera.
    SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 0.f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->SetRelativeLocation(FVector(0.f,0.f,60.f));

	// The camera follows the spring arm rotation so mouse look controls aim direction.
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>("Camera");
	PlayerCamera->SetupAttachment(SpringArm);
	PlayerCamera->bUsePawnControlRotation = false;

	Spawner = nullptr;
	WeaponSelector = nullptr;
	DefaultSecondaryWeaponClass = ATorchCPP::StaticClass();
	DefaultTertiaryWeaponClass = AHeavyAxe::StaticClass();

	GetMesh()->SetRelativeRotation(FRotator(0.f,-90.f,0.f));
}

// Called when the game starts or when spawned
void ATHCharater::BeginPlay()
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

	if (SpawnerClass && !Spawner)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.Instigator = this;
		Spawner = GetWorld()->SpawnActor<AProjectileSpawner>(SpawnerClass, GetActorLocation(), GetActorRotation(), SpawnParameters);
		if (Spawner)
		{
			Spawner->SetActorEnableCollision(false);
			// Attach to a socket when available, otherwise keep the spawner centered on the character root.
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
		// Spawn the native selector automatically unless the character Blueprint overrides it.
		UClass* SelectorClassToSpawn = WeaponSelectorClass ? WeaponSelectorClass.Get() : AWeaponselector::StaticClass();
		WeaponSelector = GetWorld()->SpawnActor<AWeaponselector>(SelectorClassToSpawn, GetActorLocation(), GetActorRotation(), SpawnParameters);
	}

	if (WeaponSelector)
	{
		WeaponSelector->InitializeWeaponSelector(
			Spawner,
			ALanceCPP::StaticClass(),
			DefaultSecondaryWeaponClass,
			DefaultTertiaryWeaponClass);
	}
}

// Called every frame
void ATHCharater::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

// Called to bind functionality to input
void ATHCharater::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Wire the Blueprint input actions to movement, look, jump, and fire behavior.
		EnhancedInputComponent->BindAction(JumpAction,ETriggerEvent::Started,this, &ATHCharater::PlayerJump);
		EnhancedInputComponent->BindAction(ShootAction,ETriggerEvent::Started,this, &ATHCharater::PlayerShoot);
		EnhancedInputComponent->BindAction(MoveAction,ETriggerEvent::Triggered,this, &ATHCharater::PlayerMove);
		EnhancedInputComponent->BindAction(LookAction,ETriggerEvent::Triggered,this, &ATHCharater::PlayerLook);
	}

	PlayerInputComponent->BindKey(EKeys::One, IE_Pressed, this, &ATHCharater::SelectPrimaryWeapon);
	PlayerInputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ATHCharater::SelectSecondaryWeapon);
	PlayerInputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ATHCharater::SelectTertiaryWeapon);
}

void ATHCharater::PlayerShoot()
{
	if (Spawner)
	{
		// Fire from the camera so the projectile follows the player's current aim point.
		const FVector SpawnLocation = PlayerCamera->GetComponentLocation() + (PlayerCamera->GetForwardVector() * 100.f) + FVector(0.f,0.f,-20.f);
		const FRotator SpawnRotation = Controller ? Controller->GetControlRotation() : PlayerCamera->GetComponentRotation();
		Spawner->Fire(SpawnLocation, SpawnRotation);
	}
}

void ATHCharater::PlayerJump()
{
	Jump();
}

void ATHCharater::PlayerMove(const FInputActionValue& ActionValue)
{
	FVector2D ActionVector = ActionValue.Get<FVector2D>();

	// Ignore pitch when moving so forward always stays parallel to the ground.
	const FRotator ControlRotation = Controller ? Controller->GetControlRotation() : GetActorRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X),ActionVector.Y);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y),ActionVector.X);
	
	//GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Emerald,TEXT("Moving"));
	
}

void ATHCharater::PlayerLook(const FInputActionValue& ActionValue)
{
	
	FVector2D ActionRotation = ActionValue.Get<FVector2D>();
	
	AddControllerYawInput(ActionRotation.X);
	AddControllerPitchInput(ActionRotation.Y);
	
}

void ATHCharater::SelectPrimaryWeapon()
{
	SelectWeaponSlot(0);
}

void ATHCharater::SelectSecondaryWeapon()
{
	SelectWeaponSlot(1);
}

void ATHCharater::SelectTertiaryWeapon()
{
	SelectWeaponSlot(2);
}

bool ATHCharater::SelectWeaponSlot(int32 SlotIndex)
{
	// The character exposes a thin wrapper so Blueprints do not need a direct selector reference.
	return WeaponSelector ? WeaponSelector->SelectWeaponSlot(SlotIndex) : false;
}

TSubclassOf<AProjectile_Base> ATHCharater::GetWeaponInSlot(int32 SlotIndex) const
{
	return WeaponSelector ? WeaponSelector->GetWeaponInSlot(SlotIndex) : nullptr;
}

int32 ATHCharater::GetActiveWeaponSlot() const
{
	return WeaponSelector ? WeaponSelector->GetActiveWeaponSlot() : INDEX_NONE;
}
