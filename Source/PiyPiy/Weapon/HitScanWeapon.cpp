// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "PiyPiy/PiyPiyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "WeaponTypes.h"
#include "PiyPiy/PiyPiyComponents/LagCompensationComponent.h"
#include "PiyPiy/PlayerController/PiyPiyPlayerController.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		APiyPiyCharacter* PiyPiyCharacter = Cast<APiyPiyCharacter>(FireHit.GetActor());
		if (PiyPiyCharacter && InstigatorController)
		{
			if (HasAuthority() && !bUseServerSideRewind || OwnerPawn->IsLocallyControlled())
			{
				UGameplayStatics::ApplyDamage(PiyPiyCharacter, Damage, InstigatorController, this, UDamageType::StaticClass());
			}
			if (!HasAuthority() && bUseServerSideRewind)
			{
				PiyPiyOwnerCharacter = PiyPiyOwnerCharacter == nullptr ? Cast<APiyPiyCharacter>(OwnerPawn) : PiyPiyOwnerCharacter;
				PiyPiyOwnerController = PiyPiyOwnerController == nullptr ? Cast<APiyPiyPlayerController>(InstigatorController) : PiyPiyOwnerController;

				if (PiyPiyOwnerCharacter && PiyPiyOwnerController && PiyPiyOwnerCharacter->GetLagCompensation() && PiyPiyOwnerCharacter->IsLocallyControlled())
				{
					PiyPiyOwnerCharacter->GetLagCompensation()->ServerScoreRequest(PiyPiyCharacter,Start,HitTarget,PiyPiyOwnerController->GetServerTime() - PiyPiyOwnerController->SingleTripTime,this);
				}
			}
		}
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
		}
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
		}
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}
	}
}
void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;
		World->LineTraceSingleByChannel(OutHit, TraceStart, End, ECollisionChannel::ECC_Visibility);
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}

		// DrawDebugSphere(GetWorld(), BeamEnd, 16.f, 12, FColor::Red, true);

		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles, TraceStart, FRotator::ZeroRotator, true);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}
