// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PiyPiyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class PIYPIY_API APiyPiyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	// Sets up properties for network replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Replication callback for score changes
	virtual void OnRep_Score() override;

	// Replication callback for defeats changes
	UFUNCTION()
	virtual void OnRep_Defeats();

	// Adds to the player's score
	void AddToScore(float ScoreAmount);

	// Adds to the number of defeats
	void AddToDefeats(int32 DefeatsAmount);

private:

	// Pointer to the character controlled by the player
	UPROPERTY()
	class APiyPiyCharacter* Character;

	// Pointer to the player controller
	UPROPERTY()
	class APiyPiyPlayerController* Controller;

	// Number of defeats
	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;
	
};
