// Copyright Epic Games, Inc. All Rights Reserved.

#include "PiyPiyCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/WidgetComponent.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"
#include "PiyPiy/Weapon/Weapon.h"
#include "PiyPiy/PiyPiyComponents/CombatComponent.h"
#include "PiyPiy/PiyPiyComponents/BuffComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "PiyPiy/Character/PiyPiyAnimInstance.h"
#include "PiyPiy//PiyPiy.h"
#include "PiyPiy/PlayerController/PiyPiyPlayerController.h"
#include "PiyPiy/GameMode/PiyPiyGameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "PiyPiy/PlayerState/PiyPiyPlayerState.h"
#include "PiyPiy/Weapon/WeaponTypes.h"
#include "Components/BoxComponent.h"
#include "PiyPiyComponents/LagCompensationComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

APiyPiyCharacter::APiyPiyCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);
	
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComponent"));

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/* Hit boxes for server-side rewind */

	Head = CreateDefaultSubobject<UBoxComponent>(TEXT("Head"));
	Head->SetupAttachment(GetMesh(), FName("head"));
	Head->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("head"), Head);

	Pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("Pelvis"));
	Pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	Pelvis->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("pelvis"), Pelvis);

	Spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine_02"));
	Spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	Spine_02->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("spine_02"), Spine_02);
	
	Spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine_03"));
	Spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	Spine_03->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("spine_03"), Spine_03);
	
	UpperArm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("UpperArm_l"));
	UpperArm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	UpperArm_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("upperarm_l"), UpperArm_l);
	
	UpperArm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("UpperArm_r"));
	UpperArm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	UpperArm_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("upperarm_r"), UpperArm_r);
	
	LowerArm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("LowerArm_l"));
	LowerArm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	LowerArm_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("lowerarm_l"), LowerArm_l);
	
	LowerArm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("LowerArm_r"));
	LowerArm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	LowerArm_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("lowerarm_r"), LowerArm_r);
	
	Hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("Hand_l"));
	Hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	Hand_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("hand_l"), Hand_l);
	
	Hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("Hand_r"));
	Hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	Hand_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("hand_r"), Hand_r);
	
	Backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("Backpack"));
	Backpack->SetupAttachment(GetMesh(), FName("backpack"));
	Backpack->SetCollisionEnabled(ECollisionEnabled::NoCollision);	
	HitCollisionBoxes.Add(FName("backpack"), Backpack);
	
	Thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("Thigh_l"));
	Thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	Thigh_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);	
	HitCollisionBoxes.Add(FName("thigh_l"), Thigh_l);
	
	Thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("Thigh_r"));
	Thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	Thigh_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);	
	HitCollisionBoxes.Add(FName("thigh_r"), Thigh_r);
	
	Calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("Calf_l"));
	Calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	Calf_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);	
	HitCollisionBoxes.Add(FName("calf_l"), Calf_l);
	
	Calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("Calf_r"));
	Calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	Calf_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);	
	HitCollisionBoxes.Add(FName("calf_r"), Calf_r);
	
	Foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("Foot_l"));
	Foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	Foot_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("foot_l"), Foot_l);
	
	Foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("Foot_r"));
	Foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	Foot_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("foot_r"), Foot_r);
}

void APiyPiyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
	if (Buff)
	{
		Buff->Character = this;
		Buff->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if (LagCompensation)
	{
		LagCompensation->Character = this;
		if (Controller)
		{
			LagCompensation->Controller = Cast<APiyPiyPlayerController>(Controller);
		}
	}
}

void APiyPiyCharacter::BeginPlay()
{  
	Super::BeginPlay();

	// Fast fix for server controller. Maybe better move to PiyPiyPlayerController later
	if (const ULocalPlayer* LocalPlayerPtr = (GEngine && GetWorld()) ? GEngine->GetFirstGamePlayer(GetWorld()) : nullptr)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayerPtr);
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	SpawnDefaultWeapon();
	UpdateHUDAmmo();
	UpdateHUDHealth();
	UpdateHUDShield();
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &APiyPiyCharacter::ReceiveDamage);
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}

void APiyPiyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);
	HideCameraIfCharacterClose();
	PollInit();
}

void APiyPiyCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void APiyPiyCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void APiyPiyCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;

	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool APiyPiyCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool APiyPiyCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* APiyPiyCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

void APiyPiyCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

FVector APiyPiyCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

bool APiyPiyCharacter::IsLocallyReloading()
{
	if (Combat == nullptr) return false;
	return Combat->bLocallyReloading;
}

ECombatState APiyPiyCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}

void APiyPiyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APiyPiyCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APiyPiyCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APiyPiyCharacter::Look);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &APiyPiyCharacter::EquipButtonPressed);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &APiyPiyCharacter::CrouchButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &APiyPiyCharacter::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &APiyPiyCharacter::AimButtonReleased);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &APiyPiyCharacter::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &APiyPiyCharacter::FireButtonReleased);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &APiyPiyCharacter::ReloadButtonPressed);
		EnhancedInputComponent->BindAction(ThrowGrenadeAction, ETriggerEvent::Started, this, &APiyPiyCharacter::GrenadeButtonPressed);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void APiyPiyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(APiyPiyCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(APiyPiyCharacter, Health);	
	DOREPLIFETIME(APiyPiyCharacter, Shield);
	DOREPLIFETIME(APiyPiyCharacter, bDisableGameplay);
}

void APiyPiyCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void APiyPiyCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void APiyPiyCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		Combat->bCanFire = false;
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
			case EWeaponType::EWT_AssaultRifle:
				SectionName = FName("Rifle");
				break;

			case EWeaponType::EWT_RocketLauncher:
				SectionName = FName("Rifle");
				break;

			case EWeaponType::EWT_Pistol:
				SectionName = FName("Rifle");
				break;
			case EWeaponType::EWT_SubmachineGun:
				SectionName = FName("Rifle");
				break;
			case EWeaponType::EWT_Shotgun:
				SectionName = FName("Shotgun");
				break;	
			case EWeaponType::EWT_SniperRifle:
				SectionName = FName("Rifle");
				break;			
			case EWeaponType::EWT_GrenadeLauncher:
				SectionName = FName("Rifle");
				break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void APiyPiyCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void APiyPiyCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void APiyPiyCharacter::Elim()
{
	DropOrDestroyWeapons();
	MulticastElim();
	GetWorldTimerManager().SetTimer(ElimTimer, this, &APiyPiyCharacter::ElimTimerFinished, ElimDelay);
}

void APiyPiyCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr) return;
	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}

void APiyPiyCharacter::DropOrDestroyWeapons()
{
	if (Combat)
	{
		if (Combat->EquippedWeapon)
		{
			DropOrDestroyWeapon(Combat->EquippedWeapon);
		}
		if (Combat->SecondaryWeapon)
		{
			DropOrDestroyWeapon(Combat->SecondaryWeapon);
		}
	}
}

void APiyPiyCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	APiyPiyGameMode* PiyPiyGameMode = Cast<APiyPiyGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = PiyPiyGameMode && PiyPiyGameMode->GetMatchState() != MatchState::InProgress;

	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

void APiyPiyCharacter::MulticastElim_Implementation()
{
	if (PiyPiyPlayerController)
	{
		PiyPiyPlayerController->SetHUDWeaponAmmo(0);
	}

	bElimmed = true;
	PlayElimMontage();

	// Start dissolve effect
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	// Disable character movement
	bDisableGameplay = true;

	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}

	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Spawn elim bot

	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ElimBotEffect, ElimBotSpawnPoint, GetActorRotation());
	}
	if (ElimBotsound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ElimBotsound, GetActorLocation());
	}
	if (IsLocallyControlled() && Combat && Combat->bAiming && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		ShowSniperScopeWidget(false);
	}
}

void APiyPiyCharacter::ElimTimerFinished()
{
	APiyPiyGameMode* PiyPiyGameMode = GetWorld()->GetAuthGameMode<APiyPiyGameMode>();
	if(PiyPiyGameMode)
	{
		PiyPiyGameMode->RequestRespawn(this, Controller);
	}
}

void APiyPiyCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void APiyPiyCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &APiyPiyCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

// Montage call play well but doesn't play in game. Need to set Slot "HitReactSlot" later after somehow
void APiyPiyCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance->IsAnyMontagePlaying()) return;
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void APiyPiyCharacter::ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	if (bElimmed) return;

	float DamageToHealth = Damage;

	if (Shield > 0.f)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
			Shield = 0.f;
		}
	}

	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);

	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();

	if (Health == 0.f)
	{
		APiyPiyGameMode* PiyPiyGameMode = GetWorld()->GetAuthGameMode<APiyPiyGameMode>();
		if (PiyPiyGameMode)
		{	
			PiyPiyPlayerController = PiyPiyPlayerController == nullptr ? Cast<APiyPiyPlayerController>(Controller) : PiyPiyPlayerController;
			APiyPiyPlayerController* AttackerController = Cast<APiyPiyPlayerController>(InstigatorController);
			PiyPiyGameMode->PlayerEliminated(this, PiyPiyPlayerController, AttackerController);
		}
	}
}

void APiyPiyCharacter::Move(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;

	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APiyPiyCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APiyPiyCharacter::EquipButtonPressed()
{
	if (bDisableGameplay) return;

	if (Combat)
	{
		ServerEquipButtonPressed();
	}
}

void APiyPiyCharacter::CrouchButtonPressed()
{
	if (bDisableGameplay) return;

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void APiyPiyCharacter::AimButtonPressed()
{
	if (bDisableGameplay) return;

	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void APiyPiyCharacter::AimButtonReleased()
{
	if (bDisableGameplay) return;

	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void APiyPiyCharacter::FireButtonPressed()
{
	if (bDisableGameplay) return;

	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void APiyPiyCharacter::FireButtonReleased()
{
	if (bDisableGameplay) return;

	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void APiyPiyCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return;

	if (Combat)
	{
		Combat->Reload();
	}
}

void APiyPiyCharacter::GrenadeButtonPressed()
{
	if (Combat)
	{
		Combat->ThrowGrenade();
	}
}

void APiyPiyCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // standing still, not jumping
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator( CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;

		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}

		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}

	if ( Speed > 0.f || bIsInAir)
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void APiyPiyCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from from [270,360] to [-90,0]
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

float APiyPiyCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void APiyPiyCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void APiyPiyCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void APiyPiyCharacter::UpdateHUDHealth()
{
	PiyPiyPlayerController = PiyPiyPlayerController == nullptr ? Cast<APiyPiyPlayerController>(Controller) : PiyPiyPlayerController;
	if (PiyPiyPlayerController)
	{
		PiyPiyPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void APiyPiyCharacter::UpdateHUDShield()
{
	PiyPiyPlayerController = PiyPiyPlayerController == nullptr ? Cast<APiyPiyPlayerController>(Controller) : PiyPiyPlayerController;
	if (PiyPiyPlayerController)
	{
		PiyPiyPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void APiyPiyCharacter::UpdateHUDAmmo()
{
	PiyPiyPlayerController = PiyPiyPlayerController == nullptr ? Cast<APiyPiyPlayerController>(Controller) : PiyPiyPlayerController;
	if (PiyPiyPlayerController && Combat && Combat->EquippedWeapon)
	{
		PiyPiyPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
		PiyPiyPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}

void APiyPiyCharacter::SpawnDefaultWeapon()
{
	APiyPiyGameMode* PiyPiyGameMode = Cast<APiyPiyGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	if (PiyPiyGameMode || World && bElimmed && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if (Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
		}

	}
}

void APiyPiyCharacter::PollInit()
{
	if (PiyPiyPlayerState == nullptr)
	{
		PiyPiyPlayerState = GetPlayerState<APiyPiyPlayerState>();
		if (PiyPiyPlayerState)
		{
			PiyPiyPlayerState->AddToScore(0.f);
			PiyPiyPlayerState->AddToDefeats(0);
			UpdateHUDHealth();
		}
	}
}

void APiyPiyCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;

	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

 }

void APiyPiyCharacter::Jump()
{
	if (bDisableGameplay) return;

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void APiyPiyCharacter::TurnInPlace(float DeltaTime)
{
	if (bDisableGameplay) return;

	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void APiyPiyCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		if (OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else if(Combat->ShouldSwapWeapons())
		{
			Combat->SwapWeapons();
		}
	}
}