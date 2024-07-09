// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "PiyPiyTypes/TurningInPlace.h"
#include "Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "PiyPiy/PiyPiyTypes/CombatState.h"
#include "PiyPiyCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class APiyPiyCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Equip Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipAction;

	/** Crouch Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	/** Aim Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	/** Reload Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;

	/** Throw grenade Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ThrowGrenadeAction;

public:
	APiyPiyCharacter();
	virtual void  Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// GetLifetimeReplicatedProps override to specify replication conditions for variables
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Give access to components
	virtual void PostInitializeComponents() override;

	// Plays the fire montage animation
	void PlayFireMontage(bool bAiming);

	// Plays the elimination montage animation
	void PlayElimMontage();

	// Plays the reload montage animation
	void PlayReloadMontage();

	// Plays the Throw grenade montage animation
	void PlayThrowGrenadeMontage();

	// Replication callback for replicated movement changes
	virtual void OnRep_ReplicatedMovement() override;

	// Multicast function to notify elimination to all clients
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	// Notifies elimination
	void Elim();

	// Called when the actor is destroyed.
	virtual void Destroyed() override;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	// Updates the health display on the HUD
	void UpdateHUDHealth();

	// Updates the shield display on the HUD
	void UpdateHUDShield();

	void UpdateHUDAmmo();

	void SpawnDefaultWeapon();

	UPROPERTY()
	TMap<FName, class UBoxComponent*> HitCollisionBoxes;

protected:

	virtual void BeginPlay();

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for equipping input */
	void EquipButtonPressed();

	/** Called for crouching input */
	void CrouchButtonPressed();

	/** Called for aiming input */
	void AimButtonPressed();
	void AimButtonReleased();

	/** Called for fire input */
	void FireButtonPressed();
	void FireButtonReleased();

	/** Called for reload input */
	void ReloadButtonPressed();

	/** Called for ThrowGrenade input */
	void GrenadeButtonPressed();

	// Handles aim offset
	void AimOffset(float DeltaTime);

	void SimProxiesTurn();

	// Overrides Jump function
	virtual void Jump() override;

	// Plays the hit react montage animation
	void PlayHitReactMontage();

	// Handles receiving damage
	UFUNCTION()
	void ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	//Poll for any relevant classes and initialize the HUD
	void PollInit();

	// Rotates the character in place
	void RotateInPlace(float DeltaTime);

	void DropOrDestroyWeapon(AWeapon* Weapon);

	void DropOrDestroyWeapons();

	/* Hit boxes used for server-side rewind */

	UPROPERTY(EditAnywhere)
	class UBoxComponent* Head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Spine_02;	
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* Spine_03;

	UPROPERTY(EditAnywhere)
	UBoxComponent* UpperArm_l;	
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* UpperArm_r;	
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* LowerArm_l;	
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* LowerArm_r;	
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* Hand_l;	
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* Hand_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Backpack;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Thigh_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Thigh_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Calf_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Calf_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Foot_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Foot_r;

private:
	// Widget component for overhead display
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	// Pointer to the currently overlapping weapon
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	// Replication callback for overlapping weapon show and hide widget
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	/* Components */

	// Combat component reference
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

	// Buff component reference
	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;

	// Lag Compensation component reference
	UPROPERTY(VisibleAnywhere)
	class ULagCompensationComponent* LagCompensation;

	// Server-side function to handle equip button press
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	// Yaw rotation for aim offset
	float AO_Yaw = 0.f;

	// Pitch rotation for aim offset
	float AO_Pitch = 0.f;

	// Interpolated yaw rotation for aim offset
	float InterpAO_Yaw = 0.f;

	// Initial aim rotation
	FRotator StartingAimRotation;

	// Turning in place state
	ETurningInPlace TurningInPlace;

	// Performs turning in place animation
	void TurnInPlace(float DeltaTime);

	// Montage for firing weapon
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	// Montage for hit react
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	// Montage for elimination react
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;

	// Montage for reload react
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;

	// Montage for throw grenade react
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ThrowGrenadeMontage;

	// Hides camera if character is close to a weapon
	void HideCameraIfCharacterClose();

	// Distance threshold for hiding camera
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	// Indicates if the root bone should rotate
	bool bRotateRootBone = false;

	// Calculates the aim offset pitch
	void CalculateAO_Pitch();

	// Threshold for turning
	float TurnThreshold = 0.5f;

	// Rotation of the root bone in the last frame
	FRotator ProxyRotationLastFrame;

	// Current rotation of the root bone
	FRotator ProxyRotation;

	// Yaw rotation of the proxy
	float ProxyYaw;

	// Time since the last movement replication
	float TimeSinceLastMovementReplication;

	// Calculates the character speed
	float CalculateSpeed();

	/** Player health */

	// Maximum health of the player
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	// Current health of the player
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	/* Player shield */

	// Maximum shield of the player
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;

	// Current shield of the player
	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats")
	float Shield = 0.f;

	// Replication callback for shield changes
	UFUNCTION()
	void OnRep_Shield(float LastShield);

	// Replication callback for health changes
	UFUNCTION()
	void OnRep_Health(float LastHealth);

	// Pointer to the player controller
	UPROPERTY()
	class APiyPiyPlayerController* PiyPiyPlayerController;

	// Indicates if the player is eliminated
	bool bElimmed = false;

	// Timer handle for elimination delay
	FTimerHandle ElimTimer;

	// Delay before elimination
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	// Called when the elimination timer finishes
	void ElimTimerFinished();

	/** Dissolve effect */

	// Timeline component for the dissolve effect
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	// Delegate for the dissolve timeline
	FOnTimelineFloat DissolveTrack;

	// Updates the dissolve material parameter
	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	// Starts the dissolve effect
	void StartDissolve();

	// Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = "Elim")
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	// Material instance set on the blueprint, used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = "Elim")
	UMaterialInstance* DissolveMaterialInstance;

	// Curve used for the dissolve effect
	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	/** Elim bot */

	// Particle system for the elimination bot effect
	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	// Component for the elimination bot particle system
	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	// Sound cue for the elimination bot
	UPROPERTY(EditAnywhere)
	class USoundCue* ElimBotsound;

	// Inputs check
	bool bInputsSet = false;

	/* Grenade */

	UPROPERTY(VisibleAnywhere, Category = "Grenade")
	UStaticMeshComponent* AttachedGrenade;

	/* Default weapon */

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;

public:

	// Getter for CameraBoom component
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	// Getter for FollowCamera component
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// Sets the currently overlapping weapon
	void SetOverlappingWeapon(AWeapon* Weapon);

	// Checks if a weapon is equipped
	bool IsWeaponEquipped();

	// Checks if the character is aiming
	bool IsAiming();

	// Getter for AO_Yaw
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }

	// Getter for AO_Pitch
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }

	// Getter for the equipped weapon
	AWeapon* GetEquippedWeapon();

	// Getter for the turning in place state
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }

	// Returns whether the root bone should rotate
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }

	// Returns whether the character is eliminated
	FORCEINLINE bool IsElimmed() const { return bElimmed; }

	// Returns the current health
	FORCEINLINE float GetHealth() const { return Health; }

	// Returns the maximum health
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	// Returns the current shield
	FORCEINLINE float GetShield() const { return Shield; }

	// setter for shield
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }

	// Returns the maximum health
	FORCEINLINE float GetMaxShield() const { return MaxShield; }

	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }

	// Returns the combat component
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }

	// Returns the buff component
	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }

	// Returns the lag compensation component
	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensation; }

	// Returns whether gameplay is disabled
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }

	// Returns Reload montage
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }

	// Returns grenade mesh
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }

	bool IsLocallyReloading();

	// Gets the current combat state
	ECombatState GetCombatState() const;

	// Getter for the hit target
	FVector GetHitTarget() const;

	// Pointer to the player state
	UPROPERTY()
	class APiyPiyPlayerState* PiyPiyPlayerState;

	// Indicates whether gameplay is disabled
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;
};

