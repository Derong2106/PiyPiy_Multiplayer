// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PiyPiyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PIYPIY_API APiyPiyPlayerController : public APlayerController
{
	GENERATED_BODY()

public: 
	// Sets the health display on the HUD
	void SetHUDHealth(float Health, float MaxHealth);

	// Sets the shield display on the HUD
	void SetHUDShield(float Shield, float MaxShield);

	// Sets the score display on the HUD
	void SetHUDScore(float Score);

	// Sets the defeats count display on the HUD
	void SetHUDDefeats(int32 Defeats);

	// Sets the weapon ammo display on the HUD
	void SetHUDWeaponAmmo(int32 Ammo);

	// Sets the carried ammo display on the HUD
	void SetHUDCarriedAmmo(int32 Ammo);

	// Sets the match countdown display on the HUD
	void SetHUDMatchCountdown(float CountdownTime);

	// Sets the announcement countdown display on the HUD
	void SetHUDAnnouncementCountdown(float CountdownTime);

	// Sets grenades count on the HUD
	void SetHUDGrenades(int32 Grenades);

	// Called when the character is possessed
	virtual void OnPossess(APawn* InPawn) override;

	virtual void Tick(float DeltaTime) override;

	// Synced with server world clock
	virtual float GetServerTime();

	// Sync with server clock as soon as possible
	virtual void ReceivedPlayer() override;

	// GetLifetimeReplicatedProps override to specify replication conditions for variables
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called when the match state is set
	void OnMatchStateSet(FName State);

	// Handles actions when the match has started
	void HandleMatchHasStarted();

	// Handles actions during the cooldown phase
	void HandleCooldown();

	void OnCharacterOverlayAddedToViewport();

	float SingleTripTime = 0.f;

protected:

	virtual void BeginPlay() override;

	// Sets the time display on the HUD
	void SetHUDTime();

	// Polls initialization processes
	void PollInit();

	/** Sync time between client and server */

	// Requests the current server time, passing in the client's time when the requst was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	//Difference between client and server time
	float ClientServerDelta = 0.f;

	// Frequency for time synchronization
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	// Running time for time synchronization
	float TimeSyncRunningTime = 0.f;

	// Checks for time synchronization
	void CheckTimeSync(float DeltaTime);

	// Server-side function to check match state
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	// Client-side function to join a midgame match
	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);

	void HighPingWarning();

	void StopHighPingWarning();

	void CheckPing(float DeltaTime);

private:
	// Instance of the game's HUD
	UPROPERTY()
	class APiyPiyHUD* PiyPiyHUD;

	// Instance of the game mode
	UPROPERTY()
	class APiyPiyGameMode* PiyPiyGameMode;
	
	// Starting time of the level
	float LevelStartingTime = 0.f;

	// Total match time
	float MatchTime = 120.f;

	// Warmup time before the match starts
	float WarmupTime = 0.f;

	// Cooldown time after the match ends
	float CooldownTime = 0.f;

	// Countdown integer
	uint32 CountdownInt = 0;

	// Current state of the match
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	// Replication callback for match state changes
	UFUNCTION()
	void OnRep_MatchState();

	// Instance of the character overlay widget
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	// HUD health and max health values
	float HUDHealth;
	bool bInitializeHealth = false;
	float HUDMaxHealth;

	// HUD shield and max shield values
	float HUDShield;
	bool bInitializeShield = false;
	float HUDMaxShield;

	// HUD score and number of defeats
	float HUDScore;
	bool bInitializeScore = false;
	int32 HUDDefeats;
	bool bInitializeDefeats = false;

	// HUD grenades amount
	int32 HUDGrenades;
	bool bInitializeGrenades = false;

	float HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;

	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;

	float HighPingRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;

	float PingAnimationRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;
};
