// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/HitResult.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Sound/SoundBase.h"
#include "Projectile_Base.generated.h"

class UPrimitiveComponent;
class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class GNG_API AProjectile_Base : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile_Base();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Handles pawn-style overlap hits and applies direct damage before destroying the projectile.
	UFUNCTION()
	virtual void OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Handles blocking impacts such as walls or props that should stop the projectile immediately.
	UFUNCTION()
	virtual void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// Root collision sized for a flat stick-like projectile.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	UBoxComponent* CollisionBox;

	// Visual mesh that child Blueprints can replace without touching collision.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	UStaticMeshComponent* ProjectileMesh;

	// Shared movement component so each child projectile can tune speed and behavior in Blueprint.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	UProjectileMovementComponent* ProjectileMovement;

	// Base direct-hit damage applied by the default overlap implementation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Damage = 25.f;

	UPROPERTY(EditAnywhere, Category = "Sounds")
	USoundBase* ThrowSound;	
	
	UPROPERTY(EditAnywhere, Category = "Sounds")
	USoundBase* HitSounds;
	
};
