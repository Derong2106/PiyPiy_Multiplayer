#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "PiyPiy/Weapon/WeaponTypes.h"
#include "AmmoPickup.generated.h"

UCLASS()
class PIYPIY_API AAmmoPickup : public APickup
{
	GENERATED_BODY()
	
protected:

	//Sphere overlap
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& Sweepresult);

private:
	
	UPROPERTY(EditAnywhere)
	int32 AmmoAmount = 30;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
};
