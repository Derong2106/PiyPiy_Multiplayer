// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "PiyPiyGameState.generated.h"

/**
 * 
 */
UCLASS()
class PIYPIY_API APiyPiyGameState : public AGameState
{
	GENERATED_BODY()
	
public:

	// Sets up properties for network replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Updates the top score and the list of top scoring players
	void UpdateTopScore(class APiyPiyPlayerState* ScoringPlayer);

	// Array of players with the top scores, replicated across the network
	UPROPERTY(Replicated)
	TArray<APiyPiyPlayerState*> TopScoringPlayers;

private:
	// Stores the highest score achieved
	float TopScore = 0.f;
};
