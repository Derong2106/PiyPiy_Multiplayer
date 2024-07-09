#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PIYPIY_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBuffComponent();

	friend class APiyPiyCharacter;

	void Heal(float HealAmount, float HealingTime);

	void ReplenishShield(float ShieldAmount, float ReplenishTime);

	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);

	void BuffJump(float BuffJumpVelocity, float BuffTime);

	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);

	void SetInitialJumpVelocity(float Velocity);

protected:
	virtual void BeginPlay() override;

	void HealRampUp(float DeltaTime);

	void ShieldRampUp(float DeltaTime);

private:

	UPROPERTY()
	class APiyPiyCharacter* Character;

	/* Heal buff */

	bool bHealing = false;

	float HealingRate = 0.f;

	float AmountToHeal = 0.f;

	/* Shield buff */

	bool bReplenishingShield = false;

	float ShieldReplenishRate = 0;

	float ShieldReplenishAmount = 0.f;

	/* Speed buff */

	FTimerHandle SpeedBuffTimer;

	void ResetSpeeds();

	float InitialBaseSpeed;

	float InitialCrouchSpeed;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);

	/* Jump Buff */

	FTimerHandle JumpBuffTimer;

	void ResetJump();

	float InitialJumpVelocity = 0.f;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};
