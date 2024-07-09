// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class PIYPIY_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	// UI element to display the health bar
	UPROPERTY( meta = (BindWidget))
	class UProgressBar* HealthBar;

	// UI element to display the health text
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;

	// UI element to display the shield bar
	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;

	// UI element to display the shield text
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;

	// UI element to display the score amount
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount;

	// UI element to display the number of defeats
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsAmount;

	// UI element to display the weapon ammo amount
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoAmount;

	// UI element to display the carried ammo amount
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoAmount;

	// UI element to display the match countdown text
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountdownText;

	// UI element to display the Grenades count text
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GrenadesText;

	// UI element to display high ping image
	UPROPERTY(meta = (BindWidget))
	class UImage* HighPingImage;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* HighPingAnimation;
};
