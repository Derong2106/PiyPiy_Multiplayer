// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PiyPiyHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:

	UPROPERTY()
	class UTexture2D* CrosshairsCenter;

	UPROPERTY()
	UTexture2D* CrosshairsLeft;

	UPROPERTY()
	UTexture2D* CrosshairsRight;

	UPROPERTY()
	UTexture2D* CrosshairsTop;

	UPROPERTY()
	UTexture2D* CrosshairsBottom;

	float CrosshairSpread;
	FLinearColor CrosshairColor;
};

UCLASS()
class PIYPIY_API APiyPiyHUD : public AHUD
{
	GENERATED_BODY()
	
public:

	// Overrides the DrawHUD function from the parent class
	virtual void DrawHUD() override;

	// Class of the character overlay widget
	UPROPERTY(Editanywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;

	// Instance of the character overlay widget
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	// Sets the HUD package data
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }

	// Adds the character overlay to the HUD
	void AddCharacterOverlay();

	// Class of the announcement widget
	UPROPERTY(Editanywhere, Category = "Announcements")
	TSubclassOf<UUserWidget> AnnouncementClass;

	// Instance of the announcement widget
	UPROPERTY()
	class UAnnouncement* Announcement;

	// Add the announcement to the HUD
	void AddAnnouncement();

protected:

	virtual void BeginPlay() override;

private:

	// Data structure containing HUD information
	UPROPERTY()
	FHUDPackage HUDPackage;

	// Draws the crosshair on the screen
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);

	// Maximum spread of the crosshair
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;


};
