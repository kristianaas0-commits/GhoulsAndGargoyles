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
	
	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
		);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category="Projectile")
    float Damage;
	
	UPROPERTY(VisibleAnywhere, EditAnywhere)
	UMeshComponent* Mesh;
	
	UPROPERTY(EditAnywhere, VisibleAnywhere)
	USphereComponent* CollisionSphere;
	
	UPROPERTY(EditAnywhere, VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovement;
};
