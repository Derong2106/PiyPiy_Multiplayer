#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PiyPiy/HUD/PiyPiyHUD.h"
#include "PiyPiy/Weapon/WeaponTypes.h"
#include "PiyPiy/PiyPiyTypes/CombatState.h"
#include "CombatComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PIYPIY_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	//Give access to character
	friend class APiyPiyCharacter;

	// Virtual function to specify replicated properties for networking
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Handles firing button input
	void FireButtonPressed(bool bPressed);

public:

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Equips a weapon
	void EquipWeapon(class AWeapon* WeaponToEquip);

	//Reload a weapon
	void Reload();

	// Completes the reloading process
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	void JumpToShotgunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	void SwapWeapons();

	bool bLocallyReloading = false;

private:

	// Character ref
	UPROPERTY()
	class APiyPiyCharacter* Character;

	// Reference to the player controller
	UPROPERTY()
	class APiyPiyPlayerController* Controller;

	// Reference to the HUD
	UPROPERTY()
	class APiyPiyHUD* HUD;

	//Equipped weapon ref
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	// Flag indicating aiming state
	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;

	bool bAimButtonPressed = false;

	UFUNCTION()
	void OnRep_Aiming();

	// Base walk speed
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed = 600.f;

	// Walk speed while aiming
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed = 450.f;

	// Flag indicating firing button state
	bool bFireButtonPressed;

	/** HUD and crosshairs */

	// Crosshair-related factors
	float CrosshairVelocityFactor;

	float CrosshairInAirFactor;

	float CrosshairAimFactor;

	float CrosshairShootingFactor;

	// Stores the location of the hit target
	FVector HitTarget;

	// Package containing HUD data
	FHUDPackage HUDPackage;

	/** Aiming and FOV */

	// Field of view when not aiming; set to the camera's base FOV in BeginPlay
	float DefaultFOV;

	// Field of view when aiming
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	// Current field of view
	float CurrentFOV;

	// Interpolation speed for zooming
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	// Interpolates field of view for smooth zoom
	void InterpFOV(float DeltaTime);

	/** Automatic fire */

	// Timer handle for firing
	FTimerHandle FireTimer;

	// Indicates whether the character can fire
	UPROPERTY(EditAnywhere, Category = Combat)
	bool bCanFire = true;

	// Starts the fire timer
	void StartFireTimer();

	// Called when the fire timer is finished
	void FireTimerFinished();

	// Initiates the firing action
	void Fire();

	void FireProjectileWeapon();

	void FireHitScanWeapon();

	void FireShotgun();

	void LocalFire(const FVector_NetQuantize& TraceHitTarget);

	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	// Checks if the character can fire
	bool CanFire();

	// Carried ammo for the currently equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	// Replication callback for carried ammo
	UFUNCTION()
	void OnRep_CarriedAmmo();

	// Map storing the carried ammo for each weapon type
	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 500;

	// Starting ammo for the assault rifle
	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 StartingARAmmo = 30;

	// Starting ammo for the rocket launcher
	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 StartingRocketAmmo = 5;

	// Starting ammo for the pistol
	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 StartingPistolAmmo = 20;

	// Starting ammo for the SMG
	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 StartingSMGAmmo = 50;

	// Starting ammo for the Shotgun
	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 StartingShotgunAmmo = 4;

	// Starting ammo for the Sniper rifle
	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 StartingSniperRifleAmmo = 5;

	// Starting ammo for the Grenade launcher
	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 StartingGrenadeLauncherAmmo = 4;

	// Initializes the carried ammo
	void InitializeCarriedAmmo();

	// Combat state of the character
	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	// Replication callback for combat state
	UFUNCTION()
	void OnRep_CombatState();

	// Updates ammo values
	void UpdateAmmoValues();

	void UpdateShotgunAmmoValues();

	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 4;

	UFUNCTION()
	void OnRep_Grenades();

	UPROPERTY(EditAnywhere, Category = "Grenade")
	int32 MaxGrenades = 4;

	void UpdateHUDGrenades();

	FORCEINLINE int32 GetGrenades() const { return Grenades; }

	bool ShouldSwapWeapons();

protected:

	virtual void BeginPlay() override;

	// Sets the aiming state
	void SetAiming(bool bIsAiming);

	// Server-side function to set aiming state
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	// Replication callback for equipped weapon
	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();

	// Server-side function to handle firing
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	// Multicast function to handle firing effects
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	// Traces the environment under the crosshairs
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	// Updates the crosshair position on the HUD
	void SetHUDCrosshairs(float DeltaTime);

	// Reloads on the server side
	UFUNCTION(Server, Reliable)
	void ServerReload();

	// Handles the reloading logic
	void HandleReload();

	// Determines the amount to reload
	int32 AmountToReload();

	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;

	void DropEquippedWeapon();

	void AttachActorToRightHand(AActor* ActorToAttach);

	void AttachActorToBack(AActor* ActorToAttach);

	void UpdateCarriedAmmo();

	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);

	void ReloadEmptyWeapon();

	void AttachActorToLeftHand(AActor* ActorToAttach);

	void ShowAttachedGrenade(bool bShowGrenade);

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);

	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);


};
