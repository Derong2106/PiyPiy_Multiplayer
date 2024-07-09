// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Announcement.generated.h"

/**
 * 
 */
UCLASS()
class PIYPIY_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:
	// UI element to display the warmup time
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WarmupTime;

	// UI element to display announcement text
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AnnouncementText;

	// UI element to display informational text
	UPROPERTY(meta = (BindWidget))
	UTextBlock* InfoText;
	
};
