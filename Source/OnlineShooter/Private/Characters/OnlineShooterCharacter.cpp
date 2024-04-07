#include "Characters/OnlineShooterCharacter.h"

// INPUT
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

// Replication
#include "Net/UnrealNetwork.h"

// References
#include "Weapon/Weapon.h"
#include "OnlineShooter.h"
#include "PlayerController/OnlineShooterPlayerController.h"

// Add libs
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Components
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CombatComponent.h"
#include "Particles/ParticleSystemComponent.h"

// GameModes
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/BuffComponent.h"
#include "Components/LagCompensationComponent.h"
#include "GameModes/OnlineShooterGameMode.h"
#include "GameStates/OnlineShooterGameState.h"
#include "PlayerStates/OnlineShooterPlayerState.h"

// Constructor
AOnlineShooterCharacter::AOnlineShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	GetMovementComponent()->NavAgentProps.bCanCrouch = true;
	
	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh(), TEXT("SKT_Camera")); 
	
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);
	
	// Create Overhead Widget
	/*OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);*/

	// Create Combat Component
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	// Buff
	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	// Collision preset
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("SKT_Grenade"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// Timeline component
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	// Set Turning State
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	// Net update settings
	NetUpdateFrequency = 120.f;
	MinNetUpdateFrequency = 66.f;
	
	// Initializing server-side rewind boxes
	#pragma region SERVER-SIDE REWIND 
	
	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));
	
	/*
	 * BOXES
	 */
	
	// head
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	// pelvis
	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	// spine_02
	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	// spine_03
	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);
		
	// upperarm_l
	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	// upperarm_r
	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	// lowerarm_l
	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);	

	// lowerarm_r
	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	// hand_l
	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	// hand_r
	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	// thigh_l
	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	// thigh_r
	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	// calf_l
	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	// calf_r
	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	// foot_l
	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	// foot_r
	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	// Set collision presets
	for (auto& Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_Hitbox);
			Box.Value->SetCollisionResponseToAllChannels(ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

#pragma endregion
	
}

// Post Initialize Components 
void AOnlineShooterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Combat component valid check
	if (Combat)
	{
		Combat->Character = this;
	}

	// Buff component valid check
	if(Buff)
	{
		// Set initial speed
		Buff->Character = this;
		Buff->SetInitialSpeeds(
			GetCharacterMovement()->MaxWalkSpeed,
			GetCharacterMovement()->MaxWalkSpeedCrouched);

		// Set initial Jump velocity
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity); 
	}

	// Buff component valid check
	if(LagCompensation)
	{
		LagCompensation->Character = this;

		if (Controller)
		{
			LagCompensation->Controller = Cast<AOnlineShooterPlayerController>(Controller);	
		}
		
	}
}

// Begin Play
void AOnlineShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Set Enhanced input subsystem
	if (const ULocalPlayer* Player = (GEngine && GetWorld()) ? GEngine->GetFirstGamePlayer(GetWorld()) : nullptr)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Player))
		{
			if (CharacterMappingContext)
			{
				Subsystem->AddMappingContext(CharacterMappingContext, 0);
			}
		}
	}
	
	//Add Input Mapping Context
	/*if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(CharacterMappingContext, 0);
		}
	}*/
	
	// Spawn default weapon
	SpawnDefaultWeapon();

	// Set Ammo amount
	UpdateHUDAmmo();
	
	// Set Health amount
	UpdateHUDHealth();

	// Set Shield amount
	UpdateHUDShield();
	
	// Set Grenades amount
	UpdateHUDGrenades();
	
	// Bind delegates only on server
	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AOnlineShooterCharacter::ReceiveDamage);
	}

	// Hide grenade mesh by default
	if(AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}

// Tick
void AOnlineShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);
	
	HideMesh();

	// Try to Initialize relevant classes every tick if it's equal to null 
	PollInit();
}

// Replication
void AOnlineShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(AOnlineShooterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AOnlineShooterCharacter, Health);
	DOREPLIFETIME(AOnlineShooterCharacter, Shield);
	DOREPLIFETIME(AOnlineShooterCharacter, bDisableGameplay);
	DOREPLIFETIME(AOnlineShooterCharacter, DamageDirection);
}

// Replicated movement
void AOnlineShooterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		SimProxiesTurn();
	}

	TimeSinceLastMovementReplication = 0.f;
}

// Try to Initialize
void AOnlineShooterCharacter::PollInit()
{
	if (!OnlineShooterPlayerState)
	{
		OnlineShooterPlayerState = GetPlayerState<AOnlineShooterPlayerState>();

		if (OnlineShooterPlayerState)
		{
			OnlineShooterPlayerState->AddToScore(0.f);
			OnlineShooterPlayerState->AddToDefeats(0);

			SetTeamColor(OnlineShooterPlayerState->GetTeam());
			
			AOnlineShooterGameState* OnlineShooterGameState = Cast<AOnlineShooterGameState>(UGameplayStatics::GetGameState(this));
			if (OnlineShooterGameState && OnlineShooterGameState->TopScoringPlayers.Contains(OnlineShooterPlayerState))
			{
				MulticastGainedTheLead();
			}
		}
	}
}

// Binding inputs
void AOnlineShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOnlineShooterCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOnlineShooterCharacter::Look);
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AOnlineShooterCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AOnlineShooterCharacter::StopJumping);	
		
		// Crouching
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AOnlineShooterCharacter::CrouchButtonPressed);
		
		// Equip
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &AOnlineShooterCharacter::EquipButtonPressed);

		// Aiming
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AOnlineShooterCharacter::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AOnlineShooterCharacter::AimButtonReleased);

		// Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AOnlineShooterCharacter::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AOnlineShooterCharacter::FireButtonReleased);

		// Reload
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AOnlineShooterCharacter::ReloadButtonPressed);

		// Throw Grenade
		EnhancedInputComponent->BindAction(ThrowGrenadeAction, ETriggerEvent::Triggered, this, &AOnlineShooterCharacter::GrenadeButtonPressed);
	}
}

// Move
void AOnlineShooterCharacter::Move(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;
	
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

// Look
void AOnlineShooterCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	bool bScopeLook =
		Combat &&
		Combat->bAiming &&
		GetEquippedWeapon() &&
		GetEquippedWeapon()->GetWeaponType() == EWeaponType::EWT_SniperRifle;

	float MouseSensitivity = bScopeLook ? .1f : 1.f;
	
	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
	 	AddControllerYawInput(LookAxisVector.X * MouseSensitivity);
		AddControllerPitchInput(LookAxisVector.Y * MouseSensitivity);
	}
}

// Jump
void AOnlineShooterCharacter::Jump()
{
	if (bDisableGameplay) return; 
	bIsCrouched ? UnCrouch() : Super::Jump();
}

// Crouch
void AOnlineShooterCharacter::CrouchButtonPressed()
{
	if (bDisableGameplay) return;
	bIsCrouched ? UnCrouch() : Crouch();
}
void AOnlineShooterCharacter::CrouchButtonReleased()
{
	if (bDisableGameplay) return;
	UnCrouch();
}

#pragma region FIRE

void AOnlineShooterCharacter::FireButtonPressed()
{
	if (bDisableGameplay) return;
	
	if(Combat && Combat->EquippedWeapon)
	{
		Combat->FireButtonPressed(true);
	}
}
void AOnlineShooterCharacter::FireButtonReleased()
{
	if (bDisableGameplay) return;
	
	if(Combat && Combat->EquippedWeapon)
	{
		Combat->FireButtonPressed(false);
	}
}

#pragma endregion
#pragma region AIM

void AOnlineShooterCharacter::AimButtonPressed()
{
	if (bDisableGameplay) return;
	
	if(Combat)
	{
		Combat->SetAiming(true);
	}
}
void AOnlineShooterCharacter::AimButtonReleased()
{
	if (bDisableGameplay) return;
	
	if(Combat && Combat->IsAiming())
	{
		Combat->SetAiming(false);
	}
}

void AOnlineShooterCharacter::AimOffset(float DeltaTime)
{
	if (!Combat && !Combat->EquippedWeapon) return;
	
	float Speed = GetVelocity().Size2D();
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	
	if (!Speed && !bIsInAir) // standing still, not jumping
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		//bUseControllerRotationYaw = false;
	
		if(TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}

		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed || bIsInAir) // running or jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void AOnlineShooterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if(AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from  [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);

		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

#pragma endregion
#pragma region TAKEN DAMAGE

void AOnlineShooterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	OnlineShooterGameMode = !OnlineShooterGameMode ? GetWorld()->GetAuthGameMode<AOnlineShooterGameMode>() : OnlineShooterGameMode;
	if(bEliminated || !OnlineShooterGameMode) return;
	
	Damage = OnlineShooterGameMode->CalculateDamage(InstigatorController, Controller, Damage);
	
	FRotator PlayerRotation = GetActorRotation();
	FRotator DamageVectorRotation = UKismetMathLibrary::MakeRotFromX(DamageCauser->GetActorLocation() - GetActorLocation());
	DamageRotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(DamageVectorRotation, PlayerRotation);
	DamageDirection = DamageRotationDelta.Yaw;
	
	// DEBUG
	if (bDebug)
	{
		UE_LOG(LogTemp, Error, TEXT("DamageRotationDelta: %f"),  DamageDirection);	
	}
	
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
	
	if(Health <= 0.f)
	{	
		OnlineShooterGameMode = !OnlineShooterGameMode ? GetWorld()->GetAuthGameMode<AOnlineShooterGameMode>() : OnlineShooterGameMode; 
		if(OnlineShooterGameMode)
		{
			OnlineShooterPlayerController = !OnlineShooterPlayerController ? Cast<AOnlineShooterPlayerController>(Controller) : OnlineShooterPlayerController;
			AOnlineShooterPlayerController* AttackerController = Cast<AOnlineShooterPlayerController>(InstigatorController);
			
			OnlineShooterGameMode->PlayerEliminated(this, OnlineShooterPlayerController, AttackerController);
		}	
	}
}

void AOnlineShooterCharacter::MultiCastHit_Implementation()
{
	PlayHitReactMontage();	
}

void AOnlineShooterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();

	if(Health < LastHealth)
	{
		PlayHitReactMontage();	
	}
	
}
void AOnlineShooterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();

	if(Shield < LastShield)
	{
		PlayHitReactMontage();	
	}
}

#pragma region TEAMS

void AOnlineShooterCharacter::SetTeamColor(ETeam InTeam)
{
	
	if (!GetMesh() || !OriginalMaterial) return;
	
	switch (InTeam) {
		
	case ETeam::ET_NoTeam:
		for (uint8 i = 0; i < 2; i++)
		{
			GetMesh()->SetMaterial(i, OriginalMaterial);
		}
		
		break;
		
	case ETeam::ET_RedTeam:
		for (uint8 i = 0; i < 2; i++)
		{
			GetMesh()->SetMaterial(i, RedMaterial);
			
		}
		break;
		
	case ETeam::ET_BlueTeam:

		for (uint8 i = 0; i < 2; i++)
		{
			GetMesh()->SetMaterial(i, BlueMaterial);
		}
		break;
	
	default:
		break;
	}
}

#pragma endregion

#pragma region GAINING THE LEAD

void AOnlineShooterCharacter::MulticastGainedTheLead_Implementation()
{
	if (!CrownSystem) return;
	if (!CrownComponent)
	{
		FVector HeadLocation = GetMesh()->GetBoneLocation("head");
		
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
				CrownSystem,
				GetMesh(),
				FName(),
				//GetActorLocation() + FVector(0.f,0.f, 110.f),
				GetMesh()->GetBoneLocation("head") + FVector(0.f,0.f, 30.f),
				GetActorRotation(),
				EAttachLocation::KeepWorldPosition,
				false
			);
	}

	if (CrownComponent)
	{
		CrownComponent->Activate();
	}
}

void AOnlineShooterCharacter::MulticastLostTheLead_Implementation()
{
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}

#pragma endregion

void AOnlineShooterCharacter::UpdateHUDHealth()
{
	OnlineShooterPlayerController = !OnlineShooterPlayerController ? Cast<AOnlineShooterPlayerController>(Controller) : OnlineShooterPlayerController;

	if(OnlineShooterPlayerController)
	{
		OnlineShooterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}
void AOnlineShooterCharacter::UpdateHUDShield() 
{
	OnlineShooterPlayerController = !OnlineShooterPlayerController ? Cast<AOnlineShooterPlayerController>(Controller) : OnlineShooterPlayerController;
	
	if(OnlineShooterPlayerController)
	{
		OnlineShooterPlayerController->SetHUDShield(Shield, MaxShield);
	}
}
void AOnlineShooterCharacter::UpdateHUDAmmo()
{
	// Player controller valid check
	OnlineShooterPlayerController = !OnlineShooterPlayerController ? Cast<AOnlineShooterPlayerController>(Controller) : OnlineShooterPlayerController;
	if(OnlineShooterPlayerController && Combat)
	{

		// Character has weapon 
		if (Combat->EquippedWeapon)
		{
			// Set Carried and Weapon ammo accordingly
			OnlineShooterPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
			OnlineShooterPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
		}

		// Character has no weapon
		else 
		{
			// Set Carried and Weapon ammo to 0
			OnlineShooterPlayerController->SetHUDCarriedAmmo(uint32(0));
			OnlineShooterPlayerController->SetHUDWeaponAmmo(uint32(0));
		}
	}
}
void AOnlineShooterCharacter::UpdateHUDGrenades()
{
	// Player controller valid check
	OnlineShooterPlayerController = !OnlineShooterPlayerController ? Cast<AOnlineShooterPlayerController>(Controller) : OnlineShooterPlayerController;
	if(OnlineShooterPlayerController && Combat /*&& Combat->GetGrenades()*/)
	{
		// Update HUD Carried and Weapon ammo
		OnlineShooterPlayerController->SetHUDGrenades(Combat->GetGrenades());
	}
}

void AOnlineShooterCharacter::Eliminated(bool bPlayerLeftGame)
{
	DropOrDestroyWeapons();
	Multicast_Eliminated(bPlayerLeftGame);
}
void AOnlineShooterCharacter::Multicast_Eliminated_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	if(OnlineShooterPlayerController)
	{
		OnlineShooterPlayerController->SetHUDWeaponAmmo(0);
	}
	
	bEliminated = true;
	PlayElimMontage();

	FAttachmentTransformRules AttachmentRules(FAttachmentTransformRules::KeepRelativeTransform);
	CameraBoom->AttachToComponent(RootComponent, AttachmentRules);
	CameraBoom->SetWorldLocation(CameraBoom->GetComponentLocation() + FVector(0.f,0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
	
	// Start Dissolve effect
	if(DissolveMaterialInstance_0)
	{
		DynamicDissolveMaterialInstance_0 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance_0, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance_0);
		DynamicDissolveMaterialInstance_0->SetScalarParameterValue(TEXT("Dissolve"), .75f);
		DynamicDissolveMaterialInstance_0->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	if(DissolveMaterialInstance_1)
	{
		
		DynamicDissolveMaterialInstance_1 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance_1, this);
		GetMesh()->SetMaterial(1, DynamicDissolveMaterialInstance_1);
		DynamicDissolveMaterialInstance_1->SetScalarParameterValue(TEXT("Dissolve"), .75f);
		DynamicDissolveMaterialInstance_1->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	if(DissolveMaterialInstance_2)
	{
		
		DynamicDissolveMaterialInstance_2 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance_2, this);
		GetMesh()->SetMaterial(2, DynamicDissolveMaterialInstance_2);
		DynamicDissolveMaterialInstance_2->SetScalarParameterValue(TEXT("Dissolve"), .75f);
		DynamicDissolveMaterialInstance_2->SetScalarParameterValue(TEXT("Glow"), 200.f); 
	}
	if(DissolveMaterialInstance_3)
	{
		
		DynamicDissolveMaterialInstance_3 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance_3, this);
		GetMesh()->SetMaterial(3, DynamicDissolveMaterialInstance_3);
		DynamicDissolveMaterialInstance_3->SetScalarParameterValue(TEXT("Dissolve"), .75f);
		DynamicDissolveMaterialInstance_3->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	if(DissolveMaterialInstance_5)
	{
		DynamicDissolveMaterialInstance_5 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance_5, this);
		GetMesh()->SetMaterial(5, DynamicDissolveMaterialInstance_5);
		DynamicDissolveMaterialInstance_5->SetScalarParameterValue(TEXT("Dissolve"), .75f);
		DynamicDissolveMaterialInstance_5->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	if(DissolveMaterialInstance_6)
	{
		
		DynamicDissolveMaterialInstance_6 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance_6, this);
		GetMesh()->SetMaterial(6, DynamicDissolveMaterialInstance_6);
		DynamicDissolveMaterialInstance_6->SetScalarParameterValue(TEXT("Dissolve"), .75f);
		DynamicDissolveMaterialInstance_6->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	if(DissolveMaterialInstance_7)
	{
		
		DynamicDissolveMaterialInstance_7 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance_7, this);
		GetMesh()->SetMaterial(7, DynamicDissolveMaterialInstance_7);
		DynamicDissolveMaterialInstance_7->SetScalarParameterValue(TEXT("Dissolve"), .75f);
		DynamicDissolveMaterialInstance_7->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	if(DissolveMaterialInstance_8)
	{
		
		DynamicDissolveMaterialInstance_8 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance_8, this);
		GetMesh()->SetMaterial(8, DynamicDissolveMaterialInstance_8);
		DynamicDissolveMaterialInstance_8->SetScalarParameterValue(TEXT("Dissolve"), .75f);
		DynamicDissolveMaterialInstance_8->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	if(DissolveMaterialInstance_9)
	{
		
		DynamicDissolveMaterialInstance_9 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance_9, this);
		GetMesh()->SetMaterial(9, DynamicDissolveMaterialInstance_9);
		DynamicDissolveMaterialInstance_9->SetScalarParameterValue(TEXT("Dissolve"), .75f);
		DynamicDissolveMaterialInstance_9->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	
	StartDissolve();

	bDisableGameplay = true;
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}

	// Disable character movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	
	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/*// Show elimbot effect
	if(ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ElimBotEffect, ElimBotSpawnPoint, GetActorRotation());
	}

	// Play elimbot sound
	if(ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ElimBotSound, GetActorLocation());
	}*/

	bool bHideSniperScope =
		IsLocallyControlled() &&
		Combat &&
		Combat->bAiming &&
		Combat->EquippedWeapon &&
		Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	
	if(bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}

	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
	
	GetWorldTimerManager().SetTimer(EliminatedTimer, this, &AOnlineShooterCharacter::EliminatedTimerFinished, EliminatedDelay);
}
void AOnlineShooterCharacter::DropOrDestroyWeapons()
{
	if(Combat)
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
void AOnlineShooterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (!Weapon) return;
	
	if (Weapon->bDestroyWeapon)
	{
		Combat->EquippedWeapon->Destroy();
	}
	else
	{
		Combat->EquippedWeapon->Dropped();
	}
}
void AOnlineShooterCharacter::EliminatedTimerFinished()
{
	OnlineShooterGameMode = !OnlineShooterGameMode ? GetWorld()->GetAuthGameMode<AOnlineShooterGameMode>() : OnlineShooterGameMode; 
	if (OnlineShooterGameMode && !bLeftGame)
	{
		OnlineShooterGameMode->RequestRespawn(this, Controller);
	}

	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

#pragma endregion
#pragma region LEAVING SESSION

void AOnlineShooterCharacter::ServerLeaveGame_Implementation()
{
	OnlineShooterGameMode = !OnlineShooterGameMode ? GetWorld()->GetAuthGameMode<AOnlineShooterGameMode>() : OnlineShooterGameMode; 
	OnlineShooterPlayerState = !OnlineShooterPlayerState ? GetPlayerState<AOnlineShooterPlayerState>() : OnlineShooterPlayerState; 
	
	if (OnlineShooterGameMode && OnlineShooterPlayerState)
	{
		OnlineShooterGameMode->PlayerLeftGame(OnlineShooterPlayerState);
	}
}

#pragma endregion
#pragma region DISSOLVE EFFECT

void AOnlineShooterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if(DynamicDissolveMaterialInstance_0)
	{
		DynamicDissolveMaterialInstance_0->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}

	if(DynamicDissolveMaterialInstance_1)
	{
		DynamicDissolveMaterialInstance_1->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}

	if(DynamicDissolveMaterialInstance_2)
	{
		DynamicDissolveMaterialInstance_2->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}

	if(DynamicDissolveMaterialInstance_3)
	{
		DynamicDissolveMaterialInstance_3->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}

	if(DynamicDissolveMaterialInstance_5)
	{
		DynamicDissolveMaterialInstance_5->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}

	if(DynamicDissolveMaterialInstance_6)
	{
		DynamicDissolveMaterialInstance_6->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}

	if(DynamicDissolveMaterialInstance_7)
	{
		DynamicDissolveMaterialInstance_7->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}

	if(DynamicDissolveMaterialInstance_8) 
	{
		DynamicDissolveMaterialInstance_8->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
	
	if(DynamicDissolveMaterialInstance_9)
	{
		DynamicDissolveMaterialInstance_9->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void AOnlineShooterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &AOnlineShooterCharacter::UpdateDissolveMaterial);

	if(DissolveCurve && DissolveTimeline)
	{
		
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

#pragma endregion

void AOnlineShooterCharacter::Destroyed()
{
	Super::Destroyed();

	if(ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	OnlineShooterGameMode = !OnlineShooterGameMode ? GetWorld()->GetAuthGameMode<AOnlineShooterGameMode>() : OnlineShooterGameMode; 
	bool bMatchNotInProgress = OnlineShooterGameMode && OnlineShooterGameMode->GetMatchState() != MatchState::InProgress;
}

void AOnlineShooterCharacter::SimProxiesTurn()
{
	if(!Combat || !Combat->EquippedWeapon) return;
	bRotateRootBone = false;
	
	float Speed = GetVelocity().Size2D();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if(Speed)
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

// Reload
void AOnlineShooterCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return;
	if(Combat)
	{
		Combat->Reload();
	}
}

// Throw grenade
void AOnlineShooterCharacter::GrenadeButtonPressed()
{
	if(Combat)
	{
		Combat->ThrowGrenade();
	}
}

// Equip weapon
void AOnlineShooterCharacter::EquipButtonPressed()
{
	if (bDisableGameplay) return;
	
	// Combat component valid check
	if(Combat)
	{
		// Server Call EquipWeapon
		if (Combat->CombatState == ECombatState::ECS_Unoccupied)
		{
			Server_EquipButtonPressed();
		}

		bool bSwapWeapon =
			Combat->ShouldSwapWeapon() &&
			!HasAuthority() &&
			Combat->CombatState == ECombatState::ECS_Unoccupied &&
			!OverlappingWeapon;

		if (bSwapWeapon)
		{
			PlaySwapWeaponMontage();
			Combat->CombatState = ECombatState::ECS_SwappingWeapon;
			bFinishedSwapping = false;
		}
	}
}

// Server EquipButtonPressed
void AOnlineShooterCharacter::Server_EquipButtonPressed_Implementation()
{
	if(Combat)
	{
		if (OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else if (Combat->ShouldSwapWeapon())
		{
			Combat->SwapWeapon();
		}
	}
	
}

// Set overlapped weapon on server
void AOnlineShooterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	/*
	 * if OverlappingWeapon not null it means that we already overlapped this weapon once and it also means that the player has quit
	 * overlapping weapon and we should hide weapon pick up widget on server.  
	 */
	if (OverlappingWeapon) { OverlappingWeapon->ShowPickupWidget(false); }
	
	// It's triggers OnRep_OverlappingWeapon logic to call on owner only client (check out GetLifetimeReplicatedProps)
	OverlappingWeapon = Weapon;

	// if player controller is a host on server
	if(IsLocallyControlled())
	{
		if(OverlappingWeapon) { OverlappingWeapon->ShowPickupWidget(true); }
	}
}

// Set overlapped weapon for clients
void AOnlineShooterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	// if player overlaps weapon (Determined by Weapon.cpp callback function)
	if(OverlappingWeapon)
	{
		// ... show overlapping weapon pick up widget
		OverlappingWeapon->ShowPickupWidget(true);
	}

	/*
	 * if LastWeapon not null it means that we already overlapped this weapon once and it also means that player has stopped
	 * overlapping weapon and we should hide weapon pick up widget on client.  
	 */
	if(LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

// Hide camera if a character is too close
void AOnlineShooterCharacter::HideMesh()
{
	if (!IsLocallyControlled()) return;
	
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}

		if(Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}

		if(Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

// Returns true if character has a combat component & equipped weapon
bool AOnlineShooterCharacter::IsWeaponEquipped() const
{
	return (Combat && Combat->EquippedWeapon);
}

// Returns true if character has a combat component & aiming
bool AOnlineShooterCharacter::IsAiming() const
{
	return (Combat && Combat->bAiming);
}

// Calculate condition for character turning in place state
void AOnlineShooterCharacter::TurnInPlace(float DeltaTime)
{
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
void AOnlineShooterCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	if (GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);	
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;

		if (TimeSinceLastMovementReplication > .25f)
		{
			OnRep_ReplicatedMovement();
		}

		CalculateAO_Pitch();
	}
}

// Spawn default weapon at the start of the game 
void AOnlineShooterCharacter::SpawnDefaultWeapon()
{
	OnlineShooterGameMode = !OnlineShooterGameMode ? GetWorld()->GetAuthGameMode<AOnlineShooterGameMode>() : OnlineShooterGameMode; 
	UWorld* World = GetWorld();
	if(World && OnlineShooterGameMode && !bEliminated && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if (Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
		}
	}
}

// Returns weapon that character has if combat component exists
AWeapon* AOnlineShooterCharacter::GetEquippedWeapon() const
{
	if(!Combat) return nullptr;
	return Combat->EquippedWeapon;
}

// Montages to play
void AOnlineShooterCharacter::PlayFireMontage(bool bAiming)
{
	// Play fire montage only if we have a combat component and a weapon
	if (!Combat || !Combat->EquippedWeapon) return;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	
	if(AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		//FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		FName SectionName = FName("RifleIronsight");
		
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
void AOnlineShooterCharacter::PlayReloadMontage()
{	
	// Play fire montage only if we have a combat component and a weapon
	if (!Combat || !Combat->EquippedWeapon) return;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
			case EWeaponType::EWT_AssaultRifle:
				SectionName = FName("Rifle");
				break;
			
			case EWeaponType::EWT_RocketLauncher:
				SectionName = FName("RocketLauncher");
				break;

			case EWeaponType::EWT_Pistol:
				SectionName = FName("Pistol");
				break;

			case EWeaponType::EWT_SubmachineGun:
				SectionName = FName("Pistol");
				break;

			case EWeaponType::EWT_Shotgun:
				SectionName = FName("Shotgun");
				break;

			case EWeaponType::EWT_SniperRifle:
				SectionName = FName("Rifle");
				break;

			case EWeaponType::EWT_GrenadeLauncher:
				SectionName = FName("GrenadeLauncher");
				break;
				
			case EWeaponType::EWT_MAX:
				break;
			
			default: ;
		}
		
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
void AOnlineShooterCharacter::PlayHitReactMontage()
{
	// Play hit react montage only if we have a combat component and a weapon
	if (!Combat /*|| !Combat->EquippedWeapon*/) return;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	FName SectionToPlay;
	
	if(AnimInstance && HitReactMontage && !AnimInstance->IsAnyMontagePlaying())
	{	
		SectionToPlay = FName("Front");

		// Front Hit
		if (DamageDirection >= -45.f && DamageDirection <= 45.f)
		{
			SectionToPlay = FName("Front");
		}

		// Left Hit
		if (DamageDirection < -45.f && DamageDirection > -135.f )
		{
			SectionToPlay = FName("Left");
		}

		// Back Hit
		if (DamageDirection <= -135.f && DamageDirection <= 135.f )
		{
			SectionToPlay = FName("Back");
		}

		// Right Hit
		if (DamageDirection < 135.f && DamageDirection > 45.f )
		{
			SectionToPlay = FName("Right");
		}

		if (bDebug)
		{
			UE_LOG(LogClass, Error, TEXT("HIT: %s, DamageDirection: %f"), *SectionToPlay.ToString(), DamageDirection)
		}
		
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionToPlay);
	}
}
void AOnlineShooterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	
	if(AnimInstance && EliminatedMontage)
	{
		FName SectionToPlay = EliminatedMontage->GetSectionName(FMath::FRandRange(0.f, EliminatedMontage->GetNumSections()));
		
		AnimInstance->Montage_Play(EliminatedMontage);
		AnimInstance->Montage_JumpToSection(SectionToPlay);
	}
}
void AOnlineShooterCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	
	if(AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}
void AOnlineShooterCharacter::PlaySwapWeaponMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if(AnimInstance && SwapWeaponMontage)
	{
		AnimInstance->Montage_Play(SwapWeaponMontage);
	}
}

FVector AOnlineShooterCharacter::GetHitTarget() const
{
	if (!Combat) return FVector();

	return Combat->HitTarget;
}

// Get combat state
ECombatState AOnlineShooterCharacter::GetCombatState() const
{
	if(!Combat) return ECombatState::ECS_MAX;

	return Combat->CombatState;
}
bool AOnlineShooterCharacter::IsLocallyReloading() const
{
	if (!Combat) return false;
	
	return Combat->bLocallyReloading;
}


