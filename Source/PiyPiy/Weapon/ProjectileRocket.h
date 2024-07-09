// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class PIYPIY_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileRocket();
	
	virtual void Destroyed() override;
protected:
	// Function called when this actor hits another
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	virtual void BeginPlay() override;

	// Sound cue for the rocket's looped sound
	UPROPERTY(EditAnywhere)
	USoundCue* ProjectileLoop;

	// Component for the looped sound of the rocket
	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;

	// Attenuation settings for the looping sound
	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;

	// Movement component for the rocket
	UPROPERTY(VisibleAnywhere)
	class URocketMovementComponent* RocketMovementComponent;

private:

};
