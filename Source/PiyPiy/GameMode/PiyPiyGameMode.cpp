// Fill out your copyright notice in the Description page of Project Settings.


#include "PiyPiyGameMode.h"
#include "PiyPiy/PiyPiyCharacter.h"
#include "PiyPiy/PlayerController/PiyPiyPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "PiyPiy/PlayerState/PiyPiyPlayerState.h"
#include "PiyPiy/GameState/PiyPiyGameState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

APiyPiyGameMode::APiyPiyGameMode()
{
	// bDelayedStart = true;
}

void APiyPiyGameMode::BeginPlay()
{
	Super::BeginPlay();

	// LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void APiyPiyGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//if (MatchState == MatchState::WaitingToStart)
	//{
	//	CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;

	//	if (CountdownTime <= 0.f)
	//	{
	//		StartMatch();
	//	}
	//}
	//else if (MatchState == MatchState::InProgress)
	//{
	//	CooldownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
	//	if (CountdownTime <= 0.f)
	//	{
	//		SetMatchState(MatchState::Cooldown);
	//	}
	//}
	//else if (MatchState == MatchState::Cooldown)
	//{
	//	CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
	//	if (CountdownTime <= 0.f)
	//	{
	//		RestartGame();
	//	}
	//}
}

void APiyPiyGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APiyPiyPlayerController* PiyPiyPlayer = Cast<APiyPiyPlayerController>(*It);
		if (PiyPiyPlayer)
		{
			PiyPiyPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void APiyPiyGameMode::PlayerEliminated(APiyPiyCharacter* ElimmedCharacter, APiyPiyPlayerController* VictimController, APiyPiyPlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;

	APiyPiyPlayerState* AttackerPlayerState = AttackerController ? Cast<APiyPiyPlayerState>(AttackerController->PlayerState) : nullptr;
	APiyPiyPlayerState* VictimPlayerState = VictimController ? Cast<APiyPiyPlayerState>(VictimController->PlayerState) : nullptr;

	APiyPiyGameState* PiyPiyGameState = GetGameState<APiyPiyGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && PiyPiyGameState)
	{
		AttackerPlayerState->AddToScore(1.f);
		PiyPiyGameState->UpdateTopScore(AttackerPlayerState);
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}

void APiyPiyGameMode::RequestRespawn(ACharacter* ElimedCharacter, AController* ElimedController)
{
	if (ElimedCharacter)
	{
		ElimedCharacter->Reset();
		ElimedCharacter->Destroy();
	}
	if (ElimedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimedController, PlayerStarts[Selection]);
	}
}

