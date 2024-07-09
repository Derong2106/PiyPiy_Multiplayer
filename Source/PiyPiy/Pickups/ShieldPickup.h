#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "ShieldPickup.generated.h"

UCLASS()
class PIYPIY_API AShieldPickup : public APickup
{
	GENERATED_BODY()
public:
	AShieldPickup();

protected:
	//Sphere overlap
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& Sweepresult);

private:

	UPROPERTY(EditAnywhere)
	float ShieldReplenishAmount = 100.f;

	UPROPERTY(EditAnywhere)
	float ShieldReplenishTime = 5.f;
};
