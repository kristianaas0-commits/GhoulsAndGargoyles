// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickupable_Base.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Jarl_ThirdPersonCharacter_CPP.h"

// Sets default values
APickupable_Base::APickupable_Base()
{
	// Pickups do not need per-frame work. They only wait for overlaps.
	PrimaryActorTick.bCanEverTick = false;
	
	// Simple scene root so mesh and collision can move together in child Blueprints.
	PickUpRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PickUpRoot"));
	RootComponent = PickUpRoot;
	
	// Visual representation only. Collision is handled by the separate overlap box.
	PickUpMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickUpMesh"));
	PickUpMesh->SetupAttachment(PickUpRoot);
	PickUpMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// Dedicated trigger volume for pickups so the mesh never blocks or interferes with player movement.
	UBoxComponent* PickupBox = CreateDefaultSubobject<UBoxComponent>(TEXT("PickupBox"));
	PickUpShape = PickupBox;
	PickupBox->SetupAttachment(PickUpRoot);
	PickupBox->SetBoxExtent(FVector(32.f, 32.f, 32.f));
	PickupBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	// Only the pawn channel should trigger collection.
	PickupBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupBox->SetGenerateOverlapEvents(true);
	// Route overlap events into the score pickup handler below.
	PickupBox->OnComponentBeginOverlap.AddDynamic(this, &APickupable_Base::OnPlayerEnterPickupBox);
	
}

// Called when the game starts or when spawned
void APickupable_Base::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APickupable_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickupable_Base::OnPlayerEnterPickupBox(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Pickups are only meant for the playable Jarl character.
	AJarl_ThirdPersonCharacter_CPP* PlayerCharacter = Cast<AJarl_ThirdPersonCharacter_CPP>(OtherActor);
	if (!PlayerCharacter)
	{
		return;
	}

	if (HandleScorePickup(PlayerCharacter))
	{
		Destroy();
	}
}

bool APickupable_Base::HandleScorePickup(AJarl_ThirdPersonCharacter_CPP* PlayerCharacter)
{
	if (!PlayerCharacter)
	{
		return false;
	}

	// Score handling stays centralized in the player class so HUD or future score-side effects stay in one place.
	PlayerCharacter->UpdateScore(ScoreAmount);
	return true;
}

