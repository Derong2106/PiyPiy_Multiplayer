#include "SpeedPickup.h"
#include "PiyPiy/PiyPiyCharacter.h"
#include "PiyPiy/PiyPiyComponents/BuffComponent.h"

void ASpeedPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& Sweepresult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bfromSweep, Sweepresult);

	APiyPiyCharacter* PiyPiyCharacter = Cast<APiyPiyCharacter>(OtherActor);
	if (PiyPiyCharacter)
	{
		UBuffComponent* Buff = PiyPiyCharacter->GetBuff();
		if (Buff)
		{
			Buff->BuffSpeed(BaseSpeedBuff, CrouchSpeedBuff, SpeedBuffTime);
		}
	}
	Destroy();
}
