// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class PIYPIY_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

	virtual void Tick(float DeltaTime) override;

	// Called when the actor is destroyed.
	virtual void Destroyed() override;

	// Collision box component
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Function called when this actor hits another
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	// Multicast function called when a hit occurs
	UFUNCTION(Client, Reliable)
	void MultiCast_OnHit(bool bCharacterHit);

	// Initial speed of the projectile
	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000;

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	// Particle system for impact
	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	// Sound cue for impact
	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	// Projectile movement component
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	// Particle system for rocket trail effects
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	// Component for the rocket trail effects
	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	void SpawnTrailSystem();

	void StartDestroyTimer();

	// Called when the destroy timer is finished
	void DestroyTimerFinished();

	// Mesh component
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	void ExplodeDamage();
	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;

	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;

private:

	// Trail particle system
	UPROPERTY(Editanywhere)
	UParticleSystem* Tracer;

	// Component for the tracer particle system
	UPROPERTY()
	class UParticleSystemComponent* TracerComponent;

	// Particle system for impact on obstacles
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactObstacleParticles;

	// Particle system for impact on characters
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactCharacterParticles;


	// Timer handle for destruction
	FTimerHandle DestroyTimer;

	// Time until the rocket is destroyed
	UPROPERTY(EditAnywhere)
	float DestroyTime = 1.f;

};
