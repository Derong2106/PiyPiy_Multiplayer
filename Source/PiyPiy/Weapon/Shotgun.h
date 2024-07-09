// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class PIYPIY_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()

public:

	void ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets);

	virtual void ShotgunFire(const TArray<FVector_NetQuantize>& HitTargets);

private:

	UPROPERTY(EditAnywhere, Category = "Ammo stats")
	uint32 NumberOfPellets = 10;
	
};
