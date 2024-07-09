#include "AmmoPickup.h"
#include "PiyPiy/PiyPiyCharacter.h"
#include "PiyPiy/PiyPiyComponents/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& Sweepresult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bfromSweep, Sweepresult);

	APiyPiyCharacter* PiyPiyCharacter = Cast<APiyPiyCharacter>(OtherActor);
	if (PiyPiyCharacter)
	{
		UCombatComponent* Combat = PiyPiyCharacter->GetCombat();
		if (Combat)
		{
			Combat->PickupAmmo(WeaponType, AmmoAmount);
		}
	}
	Destroy();
}
