// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

// Enum defining the states of a weapon
UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),

	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class PIYPIY_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

	FVector TraceEndWithScatter(const FVector& HitTarget);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Sphere overlap
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& Sweepresult);

	// Sphere EndOverlap
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void OnWeaponStateSet();

	virtual void OnEquipped();

	virtual void OnDropped();

	virtual void OnEquippedSecondary();

	/* Trace end with scatter */

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;

	// Weapon damage
	UPROPERTY(EditAnywhere, Category = "Weapon Stats")
	float Damage = 20.f;

	UPROPERTY(EditAnywhere)
	bool bUseServerSideRewind = false;

	// Pointer to the character who owns the weapon
	UPROPERTY()
	class APiyPiyCharacter* PiyPiyOwnerCharacter;

	// Pointer to the player controller who owns the weapon
	UPROPERTY()
	class APiyPiyPlayerController* PiyPiyOwnerController;

private:
	//Skeletal Mesh Component
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	// Sphere component
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	// Enum variable
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	// Replication callback for weapon state
	UFUNCTION()
	void OnRep_WeaponState();

	// Widget component for displaying pickup information
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	// Animation for firing the weapon
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;

	// Class of the casing to spawn
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;

	// Amount of ammo for the weapon
	UPROPERTY(EditAnywhere)
	int32 Ammo;

	// Decreases ammo count when a round is spent
	void SpendRound();

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);
	// Magazine capacity of the weapon
	UPROPERTY(EditAnywhere, Category = "Weapon Stats")
	int32 MagCapacity;

	// The number of unprocessed server requests for ammo. Incremented in SpendRound, decremented in ClientUpdateAmmo
	int32 Sequence = 0;

	// Type of the weapon
	UPROPERTY(EditAnywhere, Category = "Weapon Stats")
	EWeaponType WeaponType;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Specifies replicated properties for networking
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Replication callback for owner changes
	virtual void OnRep_Owner() override;

	// Sets the HUD ammo display
	void SetHUDAmmo();

	// Show pickup widget
	void ShowPickupWidget(bool bShowWidget);

	// Sets the state of the weapon
	void SetWeaponState(EWeaponState State);

	// Function to fire the weapon
	virtual void Fire(const FVector& HitTarget);

	// Called when the weapon is dropped
	void Dropped();

	// Adds ammo to the weapon
	void AddAmmo(int32 AmmoToAdd);

	// Getter for the weapon's sphere component
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }

	// Getter for the weapon's mesh component
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	// Getter for the zoomed FOV
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }

	// Getter for the zoom interpolation speed
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }

	// Returns the type of the weapon
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }

	// Returns the current ammo count
	FORCEINLINE int32 GetAmmo() const { return Ammo; }

	// Returns the magazine capacity
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }

	FORCEINLINE float GetDamage() const { return Damage; }

	// Checks if the weapon is empty (has no ammo)
	bool IsEmpty();

	// Checks if the weapon is full
	bool IsFull();

	/** Textures for the weapon crosshair */

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsButtom;

	/** Zoomed FOV while aiming */

	// Zoomed FOV value while aiming
	UPROPERTY(EditAnywhere, Category = "Weapon Stats")
	float ZoomedFOV = 30.f;

	// Interpolation speed for zooming
	UPROPERTY(EditAnywhere, Category = "Weapon Stats")
	float ZoomInterpSpeed = 20.f;

	/** Automatic fire */

	// Delay between consecutive shots
	UPROPERTY(EditAnywhere,Category = "Weapon Stats")
	float FireDelay = 0.15f;

	// Indicates if the weapon is automatic (can fire continuously)
	UPROPERTY(EditAnywhere, Category = "Weapon Stats")
	bool bAutomatic = true;

	// Sound cue for equipping the weapon
	UPROPERTY(Editanywhere, Category = "Weapon Stats")
	class USoundCue* EquipSound;

	/* Enable or disable custom depth */

	void EnableCustomDepth(bool bEnable);

	bool bDestroyWeapon = false;

	UPROPERTY(EditAnywhere)
	EFireType FireType;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;
};
