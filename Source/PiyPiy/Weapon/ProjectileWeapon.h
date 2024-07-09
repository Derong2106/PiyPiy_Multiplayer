// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class PIYPIY_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()
	
private:

	// Class of the projectile to spawn
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;

public:

	// Function to fire the weapon
	virtual void Fire(const FVector& HitTarget) override;
};
