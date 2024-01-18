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
#include "Components/BuffComponent.h"
#include "GameModes/OnlineShooterGameMode.h"
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
}

// Post Initialize Components 
void AOnlineShooterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (Combat)
	{
		Combat->Character = this;
	}

	if(Buff)
	{
		Buff->Character = this;
	}
}

// Begin Play
void AOnlineShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Set Enhanced input subsystem
	if (const ULocalPlayer* Player = (GEngine && GetWorld()) ? GEngine->GetFirstGamePlayer(GetWorld()) : nullptr)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Player);
		if (CharacterMappingContext)
		{
			Subsystem->AddMappingContext(CharacterMappingContext, 0);
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

	// Set Health at the start of the game
	UpdateHUDHealth();
	
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
	DOREPLIFETIME(AOnlineShooterCharacter, bDisableGameplay);
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

void AOnlineShooterCharacter::PollInit()
{
	if (!OnlineShooterPlayerState)
	{
		OnlineShooterPlayerState = GetPlayerState<AOnlineShooterPlayerState>();

		if (OnlineShooterPlayerState)
		{
			OnlineShooterPlayerState->AddToScore(0.f);
			OnlineShooterPlayerState->AddToDefeats(0);
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
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &AOnlineShooterCharacter::EquipButtonPressed);

		// Aiming
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AOnlineShooterCharacter::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AOnlineShooterCharacter::AimButtonReleased);

		// Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AOnlineShooterCharacter::FireButtonPressed);
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

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
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
	
	if(Combat)
	{
		Combat->SetAiming(false);
	}
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

#pragma region HEALTH

void AOnlineShooterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	if(bEliminated) return;
	
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();

	if(Health <= 0.f)
	{
		AOnlineShooterGameMode* OnlineShooterGameMode = GetWorld()->GetAuthGameMode<AOnlineShooterGameMode>();
		if(OnlineShooterGameMode)
		{
			OnlineShooterPlayerController = !OnlineShooterPlayerController ? Cast<AOnlineShooterPlayerController>(Controller) : OnlineShooterPlayerController;
			AOnlineShooterPlayerController* AttackerController = Cast<AOnlineShooterPlayerController>(InstigatorController);
			
			OnlineShooterGameMode->PlayerEliminated(this, OnlineShooterPlayerController, AttackerController);
		}	
	}
}

void AOnlineShooterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();

	if(Health < LastHealth)
	{
		PlayHitReactMontage();	
	}
	
}
void AOnlineShooterCharacter::UpdateHUDHealth()
{
	OnlineShooterPlayerController = !OnlineShooterPlayerController ? Cast<AOnlineShooterPlayerController>(Controller) : OnlineShooterPlayerController;

	if(OnlineShooterPlayerController)
	{
		OnlineShooterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void AOnlineShooterCharacter::Eliminated()
{
	if(Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Dropped();	
	}
	
	Multicast_Eliminated();
	GetWorldTimerManager().SetTimer(EliminatedTimer, this, &AOnlineShooterCharacter::EliminatedTimerFinished, EliminatedDelay);
}

void AOnlineShooterCharacter::Multicast_Eliminated_Implementation()
{
	if(OnlineShooterPlayerController)
	{
		OnlineShooterPlayerController->SetHUDWeaponAmmo(0);
	}
	
	bEliminated = true;
	PlayElimMontage();
	
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

	// Show elimbot effect
	if(ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ElimBotEffect, ElimBotSpawnPoint, GetActorRotation());
	}

	// Play elimbot sound
	if(ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ElimBotSound, GetActorLocation());
	}

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
}

void AOnlineShooterCharacter::EliminatedTimerFinished()
{
	AOnlineShooterGameMode* OnlineShooterGameMode = GetWorld()->GetAuthGameMode<AOnlineShooterGameMode>();
	
	if (OnlineShooterGameMode)
	{
		OnlineShooterGameMode->RequestRespawn(this, Controller);
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

	AOnlineShooterGameMode* OnlineShooterGameMode = Cast<AOnlineShooterGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = OnlineShooterGameMode && OnlineShooterGameMode->GetMatchState() != MatchState::InProgress;
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

// Fire
void AOnlineShooterCharacter::FireButtonPressed(const FInputActionInstance& InputInstance)
{
	if (bDisableGameplay) return;
	if(Combat && Combat->EquippedWeapon)
	{
		Combat->FireButtonPressed(true);
	}
}
void AOnlineShooterCharacter::FireButtonReleased(const FInputActionInstance& InputInstance)
{
	if (bDisableGameplay) return;
	if(Combat && Combat->EquippedWeapon)
	{
		Combat->FireButtonPressed(false);
	}
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
		// if we are server
		if(HasAuthority())
		{
			// call EquipWeapon on Server
			Combat->EquipWeapon(OverlappingWeapon);
		}
		// if we are client
		else
		{
			// call RPC EquipButtonPressed for clients
			Server_EquipButtonPressed();
		}
	}
}

// RPC EquipButtonPressed
void AOnlineShooterCharacter::Server_EquipButtonPressed_Implementation()
{
	if(Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

// Set overlapped weapon on server
void AOnlineShooterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	/*
	 * if OverlappingWeapon not null it means that we already overlapped this weapon once and it also means that player has quit
	 * overlapping weapon and we should hide weapon pick up widget on server.  
	 */
	if (OverlappingWeapon) { OverlappingWeapon->ShowPickupWidget(false); }
	
	// It's triggers OnRep_OverlappingWeapon logic to call on owner only client (check out GetLifetimeReplicatedProps)
	OverlappingWeapon = Weapon;

	// if player controller is a host  on server
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
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
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

		if (FMath::Abs(AO_Yaw) < 5.f)
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
		FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
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
	if (!Combat || !Combat->EquippedWeapon) return;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	
	if(AnimInstance && HitReactMontage && !AnimInstance->IsAnyMontagePlaying())
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
void AOnlineShooterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	
	if(AnimInstance && EliminatedMontage)
	{
		AnimInstance->Montage_Play(EliminatedMontage);
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







