#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "JumpPickup.generated.h"

UCLASS()
class PIYPIY_API AJumpPickup : public APickup
{
	GENERATED_BODY()
	
protected:
	//Sphere overlap
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& Sweepresult);

private:

	UPROPERTY(EditAnywhere)
	float JumpZVelocityBuff = 4000.f;

	UPROPERTY(EditAnywhere)
	float JumpBuffTime = 30.f;
};
