// Fill out your copyright notice in the Description page of Project Settings.


#include "PiyPiyPlayerController.h"
#include "PiyPiy/HUD/PiyPiyHUD.h"
#include "PiyPiy/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "PiyPiy/PiyPiyCharacter.h"
#include "Net/UnrealNetwork.h"
#include "PiyPiy/GameMode/PiyPiyGameMode.h"
#include "PiyPiy/PlayerState/PiyPiyPlayerState.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "PiyPiy/PiyPiyComponents/CombatComponent.h"
#include "PiyPiy/GameState/PiyPiyGameState.h"
#include "Components/Image.h"

void APiyPiyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//ServerCheckMatchState();

	PiyPiyHUD = Cast<APiyPiyHUD>(GetHUD());
	ServerCheckMatchState();
}

void APiyPiyPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
	CheckPing(DeltaTime);
}

void APiyPiyPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<APiyPiyPlayerState>() : PlayerState;
		if (PlayerState)
		{
			if (PlayerState->GetPingInMilliseconds() > HighPingThreshold)
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
			}
		}
		HighPingRunningTime = 0.f;
	}

	if (PiyPiyHUD && PiyPiyHUD->CharacterOverlay && PiyPiyHUD->CharacterOverlay->HighPingAnimation && PiyPiyHUD->CharacterOverlay->IsAnimationPlaying(CharacterOverlay->HighPingAnimation))
	{
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void APiyPiyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APiyPiyPlayerController, MatchState);
}

void APiyPiyPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void APiyPiyPlayerController::HighPingWarning()
{
	PiyPiyHUD = PiyPiyHUD == nullptr ? Cast<APiyPiyHUD>(GetHUD()) : PiyPiyHUD;

	bool bHUDValid = PiyPiyHUD &&
		PiyPiyHUD->CharacterOverlay &&
		PiyPiyHUD->CharacterOverlay->HighPingImage &&
		PiyPiyHUD->CharacterOverlay->HighPingAnimation;

	if (bHUDValid)
	{
		PiyPiyHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		PiyPiyHUD->CharacterOverlay->PlayAnimation(PiyPiyHUD->CharacterOverlay->HighPingAnimation,0.f,5);
	}
}

void APiyPiyPlayerController::StopHighPingWarning()
{
	PiyPiyHUD = PiyPiyHUD == nullptr ? Cast<APiyPiyHUD>(GetHUD()) : PiyPiyHUD;

	bool bHUDValid = PiyPiyHUD &&
		PiyPiyHUD->CharacterOverlay &&
		PiyPiyHUD->CharacterOverlay->HighPingImage &&
		PiyPiyHUD->CharacterOverlay->HighPingAnimation;

	if (bHUDValid)
	{
		PiyPiyHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if (PiyPiyHUD->CharacterOverlay->IsAnimationPlaying(PiyPiyHUD->CharacterOverlay->HighPingAnimation))
		{
			PiyPiyHUD->CharacterOverlay->StopAnimation(PiyPiyHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}

void APiyPiyPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (PiyPiyHUD && PiyPiyHUD->CharacterOverlay)
		{
			CharacterOverlay = PiyPiyHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				// Still have bug with ammo after elim, when HUD was created but wasn't added to viewport. OnCharacterOverlayAddedToViewport called in handleMatchHasStarted but because this system doesn't work needs to be tested after fix the match state
				OnCharacterOverlayAddedToViewport();

				APiyPiyCharacter* PiyPiyCharacter = Cast<APiyPiyCharacter>(GetPawn());
				if (PiyPiyCharacter && PiyPiyCharacter->GetCombat())
				{
					if (bInitializeGrenades)SetHUDGrenades(HUDGrenades);
				}
			}
		}
	}
}

void APiyPiyPlayerController::ServerCheckMatchState_Implementation()
{
	APiyPiyGameMode* GameMode = Cast<APiyPiyGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		//ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
	}
}

void APiyPiyPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match,float Cooldown, float StartingTime)
{
	Warmup = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	//OnMatchStateSet(MatchState);

	if (PiyPiyHUD && MatchState == MatchState::WaitingToStart)
	{
		PiyPiyHUD->AddAnnouncement();
	}
}

void APiyPiyPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void APiyPiyPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void APiyPiyPlayerController::HandleMatchHasStarted()
{
	PiyPiyHUD = PiyPiyHUD == nullptr ? Cast<APiyPiyHUD>(GetHUD()) : PiyPiyHUD;
	if (PiyPiyHUD)
	{
		//if (PiyPiyHUD->CharacterOverlay == nullptr) 
		PiyPiyHUD->AddCharacterOverlay();
		OnCharacterOverlayAddedToViewport();

		//if (PiyPiyHUD->Announcement)
		//{
		//	PiyPiyHUD->Announcement->SetVisibility(ESlateVisibility::Collapsed);
		//}
	}
}

void APiyPiyPlayerController::HandleCooldown()
{
	PiyPiyHUD = PiyPiyHUD == nullptr ? Cast<APiyPiyHUD>(GetHUD()) : PiyPiyHUD;
	if (PiyPiyHUD)
	{
		if(PiyPiyHUD->CharacterOverlay)
			PiyPiyHUD->CharacterOverlay->RemoveFromParent();
		bool bHUDValid = PiyPiyHUD->Announcement && PiyPiyHUD->Announcement->AnnouncementText && PiyPiyHUD->Announcement->InfoText;

		if (bHUDValid)
		{
			PiyPiyHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("New Match Starts In:");
			PiyPiyHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			APiyPiyGameState* PiyPiyGameState = Cast<APiyPiyGameState>(UGameplayStatics::GetGameState(this));
			APiyPiyPlayerState* PiyPiyPlayerState = GetPlayerState<APiyPiyPlayerState>();
			if (PiyPiyGameState)
			{
				TArray<APiyPiyPlayerState*> TopPlayers = PiyPiyGameState->TopScoringPlayers;
				FString InfoTextString;
				if (TopPlayers.Num() == 0)
				{
					InfoTextString = FString("There is no winner");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == PiyPiyPlayerState)
				{
					InfoTextString = FString("You are the winner!");
				}
				else if (TopPlayers.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *TopPlayers[0]->GetPlayerName());
				}
				else if (TopPlayers.Num() > 1)
				{
					InfoTextString = FString("Players tied for the win: \n");
					for (auto TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
				}
				PiyPiyHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	
	APiyPiyCharacter* PiyPiyCharacter = Cast<APiyPiyCharacter>(GetPawn());
	if (PiyPiyCharacter && PiyPiyCharacter->GetCombat())
	{
		PiyPiyCharacter->bDisableGameplay = true;
		PiyPiyCharacter->GetCombat()->FireButtonPressed(false);
	}
}

void APiyPiyPlayerController::OnCharacterOverlayAddedToViewport()
{
	if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
	if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
	if (bInitializeScore) SetHUDScore(HUDScore);
	if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);
	if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
	if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);
}

float APiyPiyPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void APiyPiyPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void APiyPiyPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	PiyPiyHUD = PiyPiyHUD == nullptr ? Cast<APiyPiyHUD>(GetHUD()) : PiyPiyHUD;

	bool bHUDValid = PiyPiyHUD && 
		PiyPiyHUD->CharacterOverlay && 
		PiyPiyHUD->CharacterOverlay->HealthBar && 
		PiyPiyHUD->CharacterOverlay->HealthText;

	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;

		PiyPiyHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		PiyPiyHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void APiyPiyPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	PiyPiyHUD = PiyPiyHUD == nullptr ? Cast<APiyPiyHUD>(GetHUD()) : PiyPiyHUD;

	bool bHUDValid = PiyPiyHUD &&
		PiyPiyHUD->CharacterOverlay &&
		PiyPiyHUD->CharacterOverlay->ShieldBar &&
		PiyPiyHUD->CharacterOverlay->ShieldText;

	if (bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;

		PiyPiyHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		PiyPiyHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void APiyPiyPlayerController::SetHUDScore(float Score)
{
	PiyPiyHUD = PiyPiyHUD == nullptr ? Cast<APiyPiyHUD>(GetHUD()) : PiyPiyHUD;
	bool bHUDValid = PiyPiyHUD &&
		PiyPiyHUD->CharacterOverlay &&
		PiyPiyHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		PiyPiyHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void APiyPiyPlayerController::SetHUDDefeats(int32 Defeats)
{
	PiyPiyHUD = PiyPiyHUD == nullptr ? Cast<APiyPiyHUD>(GetHUD()) : PiyPiyHUD;
	bool bHUDValid = PiyPiyHUD &&
		PiyPiyHUD->CharacterOverlay &&
		PiyPiyHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"),Defeats);
		PiyPiyHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void APiyPiyPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	PiyPiyHUD = PiyPiyHUD == nullptr ? Cast<APiyPiyHUD>(GetHUD()) : PiyPiyHUD;
	bool bHUDValid = PiyPiyHUD &&
		PiyPiyHUD->CharacterOverlay &&
		PiyPiyHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		PiyPiyHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void APiyPiyPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	PiyPiyHUD = PiyPiyHUD == nullptr ? Cast<APiyPiyHUD>(GetHUD()) : PiyPiyHUD;
	bool bHUDValid = PiyPiyHUD &&
		PiyPiyHUD->CharacterOverlay &&
		PiyPiyHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		PiyPiyHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void APiyPiyPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	PiyPiyHUD = PiyPiyHUD == nullptr ? Cast<APiyPiyHUD>(GetHUD()) : PiyPiyHUD;
	bool bHUDValid = PiyPiyHUD &&
		PiyPiyHUD->CharacterOverlay &&
		PiyPiyHUD->CharacterOverlay->MatchCountdownText;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			PiyPiyHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PiyPiyHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void APiyPiyPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	PiyPiyHUD = PiyPiyHUD == nullptr ? Cast<APiyPiyHUD>(GetHUD()) : PiyPiyHUD;
	bool bHUDValid = PiyPiyHUD &&
		PiyPiyHUD->Announcement &&
		PiyPiyHUD->Announcement->WarmupTime;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			PiyPiyHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PiyPiyHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void APiyPiyPlayerController::SetHUDGrenades(int32 Grenades)
{
	PiyPiyHUD = PiyPiyHUD == nullptr ? Cast<APiyPiyHUD>(GetHUD()) : PiyPiyHUD;
	bool bHUDValid = PiyPiyHUD &&
		PiyPiyHUD->CharacterOverlay &&
		PiyPiyHUD->CharacterOverlay->GrenadesText;
	if (bHUDValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		PiyPiyHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}
	else
	{
		HUDGrenades = Grenades;
	}
}

void APiyPiyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	APiyPiyCharacter* PiyPiyCharacter = Cast<APiyPiyCharacter>(InPawn);
	if (PiyPiyCharacter)
	{
		SetHUDHealth(PiyPiyCharacter->GetHealth(), PiyPiyCharacter->GetMaxHealth());
	}
}

void APiyPiyPlayerController::SetHUDTime()
{
	//float TimeLeft = 0.f;
	//if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	//else if(MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	//else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;

	//uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	//if (HasAuthority())
	//{
	//	PiyPiyGameMode = PiyPiyGameMode == nullptr ? Cast<APiyPiyGameMode>(UGameplayStatics::GetGameMode(this)) : PiyPiyGameMode;
	//	if (PiyPiyGameMode)
	//	{
	//		SecondsLeft = FMath::CeilToInt(PiyPiyGameMode->GetCountdownTime() + LevelStartingTime);
	//	}
	//}

	//if (CountdownInt != SecondsLeft)
	//{
	//	if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
	//	{
	//		SetHUDAnnouncementCountdown(TimeLeft);
	//	}
	//	if (MatchState == MatchState::InProgress)
	//	{
	//		SetHUDMatchCountdown(TimeLeft);
	//	}
	//}
	//CountdownInt = SecondsLeft;

	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());
	if (CountdownInt != SecondsLeft)
	{
		SetHUDMatchCountdown(MatchTime - GetServerTime());
	}
	CountdownInt = SecondsLeft;
}

void APiyPiyPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void APiyPiyPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}
