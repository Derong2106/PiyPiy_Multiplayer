// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

UCLASS()
class PIYPIY_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	// Function to fire the weapon
	virtual void Fire(const FVector& HitTarget) override;

protected:

	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);	
	
	// Sound cue for hit effects
	UPROPERTY(EditAnywhere, Category = "Weapon Effects")
	USoundCue* HitSound;

	// Particle system for impact effects
	UPROPERTY(EditAnywhere, Category = "Weapon Effects")
	class UParticleSystem* ImpactParticles;

private:
	// Particle system for beam effects
	UPROPERTY(EditAnywhere, Category = "Weapon Effects")
	UParticleSystem* BeamParticles;

	// Particle system for muzzle flash effects
	UPROPERTY(EditAnywhere, Category = "Weapon Effects")
	UParticleSystem* MuzzleFlash;

	// Sound cue for firing the weapon
	UPROPERTY(EditAnywhere, Category = "Weapon Effects")
	USoundCue* FireSound;

};
