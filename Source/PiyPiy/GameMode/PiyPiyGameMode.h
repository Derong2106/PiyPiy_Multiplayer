// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "PiyPiyGameMode.generated.h"

namespace MatchState
{
	extern PIYPIY_API const FName Cooldown; // Match duration has been reached. Display winner and begin cooldown timer.
}

/**
 * 
 */
UCLASS()
class PIYPIY_API APiyPiyGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:

	APiyPiyGameMode();

	virtual void Tick(float DeltaTime) override;

	// Called when a player is eliminated
	virtual void PlayerEliminated(class APiyPiyCharacter* ElimmedCharacter, class APiyPiyPlayerController* VictimController, class APiyPiyPlayerController* AttackerController);

	// Requests a respawn for the eliminated character
	virtual void RequestRespawn(class ACharacter* ElimedCharacter, AController* ElimedController);

	// Time allocated for the warmup phase in seconds
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	// Duration of the match in seconds
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	// Time allocated for the cooldown phase in seconds
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	// Stores the starting time of the level
	float LevelStartingTime = 0.f;

	// Returns the level starting time
	FORCEINLINE float GetLevelStartingTime() const { return LevelStartingTime; }

	// Returns the countdown time
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }

protected:

	virtual void BeginPlay() override;

	// Called when the match state is set
	virtual void OnMatchStateSet() override;

private:

	// Stores the countdown time
	float CountdownTime = 0.f;
};
