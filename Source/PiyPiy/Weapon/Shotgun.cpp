#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "PiyPiy/PiyPiyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "PiyPiy/PiyPiyCharacter.h"
#include "PiyPiy/PlayerController/PiyPiyPlayerController.h"
#include "PiyPiy/PiyPiyComponents/LagCompensationComponent.h"

void AShotgun::ShotgunFire(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		// Maps hit character to number of times hit
		TMap<APiyPiyCharacter*, uint32> HitMap;

		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			APiyPiyCharacter* PiyPiyCharacter = Cast<APiyPiyCharacter>(FireHit.GetActor());
			if (PiyPiyCharacter)
			{
				if (HitMap.Contains(PiyPiyCharacter))
				{
					HitMap[PiyPiyCharacter]++;
				}
				else
				{
					HitMap.Emplace(PiyPiyCharacter, 1);
				}
				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
				}
				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint, 0.5f, FMath::FRandRange(-0.5f, 0.5f));
				}
			}
		}

		TArray<APiyPiyCharacter*> HitCharacters;

		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && InstigatorController)
			{
				if (HasAuthority() && !bUseServerSideRewind)
				{
					UGameplayStatics::ApplyDamage(HitPair.Key, Damage * HitPair.Value, InstigatorController, this, UDamageType::StaticClass());
				}

				HitCharacters.Add(HitPair.Key);
			}
		}

		if (!HasAuthority() && bUseServerSideRewind)
		{
			PiyPiyOwnerCharacter = PiyPiyOwnerCharacter == nullptr ? Cast<APiyPiyCharacter>(OwnerPawn) : PiyPiyOwnerCharacter;
			PiyPiyOwnerController = PiyPiyOwnerController == nullptr ? Cast<APiyPiyPlayerController>(InstigatorController) : PiyPiyOwnerController;

			if (PiyPiyOwnerCharacter && PiyPiyOwnerController && PiyPiyOwnerCharacter->GetLagCompensation() && PiyPiyOwnerCharacter->IsLocallyControlled())
			{
				PiyPiyOwnerCharacter->GetLagCompensation()->ShotgunServerScoreRequest(HitCharacters,Start, HitTargets, PiyPiyOwnerController->GetServerTime() - PiyPiyOwnerController->SingleTripTime);
			}
		}
	}
}
void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;

		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();

		HitTargets.Add(ToEndLoc);
	}
}
