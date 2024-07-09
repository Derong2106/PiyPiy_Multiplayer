// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

UCLASS()
class PIYPIY_API ACasing : public AActor
{
	GENERATED_BODY()
	
public:	

	ACasing();

protected:

	virtual void BeginPlay() override;

	// Function called when this actor hits another
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:

	// Mesh component for the casing
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CasingMesh;

	// Impulse applied to eject the shell
	UPROPERTY(EditAnywhere)
	float ShellEjectionImpulse = 0.f;

	// Sound cue for shell ejection
	UPROPERTY(EditAnywhere)
	class USoundCue* ShellSound;

};
