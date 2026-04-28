// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "EnemyProjectile.generated.h"

UCLASS()
class GNG_API AEnemyProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemyProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// For begin overlap event
	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
		);
	
	// For delay function
	UFUNCTION()
	void DestroySelf();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// For delay function
	FTimerHandle DelayTimerHandle;
	
	UPROPERTY(EditAnywhere, Category="Projectile")
    float Damage;
	
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Mesh; // Mesh
	
	UPROPERTY(EditAnywhere)
	USphereComponent* CollisionSphere; // Collision Sphere
	
	UPROPERTY(EditAnywhere)
	UProjectileMovementComponent* ProjectileMovement; // Projectile Movement
};
