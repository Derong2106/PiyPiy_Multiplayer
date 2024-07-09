// Fill out your copyright notice in the Description page of Project Settings.


#include "PiyPiyAnimInstance.h"
#include "PiyPiy/PiyPiyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "PiyPiy/Weapon/Weapon.h"

void UPiyPiyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PiyPiyCharacter = Cast<APiyPiyCharacter>(TryGetPawnOwner());
}

void UPiyPiyAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (PiyPiyCharacter == nullptr)
	{
		PiyPiyCharacter = Cast<APiyPiyCharacter>(TryGetPawnOwner());
	}
	if (PiyPiyCharacter == nullptr) return;

	FVector Velocity = PiyPiyCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = PiyPiyCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = PiyPiyCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = PiyPiyCharacter->IsWeaponEquipped();
	bIsCrouched = PiyPiyCharacter->bIsCrouched;
	bAiming = PiyPiyCharacter->IsAiming();
	EquippedWeapon = PiyPiyCharacter->GetEquippedWeapon();
	TurningInPlace = PiyPiyCharacter->GetTurningInPlace();
	bRotateRootBone = PiyPiyCharacter->ShouldRotateRootBone();
	bElimmed = PiyPiyCharacter->IsElimmed();

	//Offset Yaw for Strafing
	FRotator AimRotation = PiyPiyCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(PiyPiyCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = PiyPiyCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = PiyPiyCharacter->GetAO_Yaw();
	AO_Pitch = PiyPiyCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && PiyPiyCharacter->GetMesh())
	{
		FVector OutPosition;
		FRotator OutRotation;

		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		PiyPiyCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (PiyPiyCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - PiyPiyCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}

		//FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		//FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		//DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f, FColor::Red);
		//DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), PiyPiyCharacter->GetHitTarget(), FColor::Green);
	}

	bUseFABRIK = PiyPiyCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
	if (PiyPiyCharacter->IsLocallyControlled() && PiyPiyCharacter->GetCombatState() != ECombatState::ECS_ThrowingGrenade)
	{
		bUseFABRIK = !PiyPiyCharacter->IsLocallyControlled();
	}
	bUseAimOffsets = PiyPiyCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !PiyPiyCharacter->GetDisableGameplay();
	bTransformRightHand = PiyPiyCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !PiyPiyCharacter->GetDisableGameplay();
}
