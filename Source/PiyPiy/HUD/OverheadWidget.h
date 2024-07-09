// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * 
 */
UCLASS()
class PIYPIY_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// Pickup Widget displaying text
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;

	// Sets text to display on the widget
	void SetDisplayText(FString TextToDisplay);

	// Show which NetRole player has
	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* InPawn);

protected:
	// Called when the UI widget is being destroyed
	virtual void NativeDestruct() override;

};
