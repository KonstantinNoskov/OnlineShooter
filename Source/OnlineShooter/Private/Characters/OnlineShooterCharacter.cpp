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

// Math
#include "Kismet/KismetMathLibrary.h"

// Components
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CombatComponent.h"


// Constructor
AOnlineShooterCharacter::AOnlineShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
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

	// Collision preset
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	//GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// Set Turning State
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	// Net update settings
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

// Replication
void AOnlineShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(AOnlineShooterCharacter, OverlappingWeapon, COND_OwnerOnly);
}

// Post Initialize Components 
void AOnlineShooterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Character = this;
	}
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

// Begin Play
void AOnlineShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(CharacterMappingContext, 0);
		}
	}
}

// Tick
void AOnlineShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	
	HideMesh();
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
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &AOnlineShooterCharacter::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AOnlineShooterCharacter::AimButtonReleased);

		// Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AOnlineShooterCharacter::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AOnlineShooterCharacter::FireButtonReleased);
	}
}

// Move
void AOnlineShooterCharacter::Move(const FInputActionValue& Value)
{
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
	bIsCrouched ? UnCrouch() : Super::Jump();
}

// Crouch
void AOnlineShooterCharacter::CrouchButtonPressed()
{
	bIsCrouched ? UnCrouch() : Crouch();
}
void AOnlineShooterCharacter::CrouchButtonReleased()
{
	UnCrouch();
}

// Aim 
void AOnlineShooterCharacter::AimButtonPressed()
{
	if(Combat)
	{
		Combat->SetAiming(true);
	}
}
void AOnlineShooterCharacter::AimButtonReleased()
{
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

void AOnlineShooterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && !Combat->EquippedWeapon) return; 
	
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

	UE_LOG(LogTemp, Warning, TEXT("ProxyYaw:%f"), ProxyYaw)
	
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
void AOnlineShooterCharacter::FireButtonPressed()
{
	if(Combat)
	{
		Combat->FireButtonPressed(true);
	}
}
void AOnlineShooterCharacter::FireButtonReleased()
{
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}
}
 
// Equip weapon
void AOnlineShooterCharacter::EquipButtonPressed()
{
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
	 * if OverlappingWeapon not null it means that we already overlapped this weapon once and it also means that player has stopped
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
void AOnlineShooterCharacter::OnRep_OverllapingWeapon(AWeapon* LastWeapon)
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

// Hide camera if character is too close
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

// Returns weapon that character has if combat component exists
AWeapon* AOnlineShooterCharacter::GetEquippedWeapon() const
{
	if(!Combat) return nullptr;
	return Combat->EquippedWeapon;
}

// Play fire montage
void AOnlineShooterCharacter::PlayFireMontage(bool bAiming)
{
	// Play fire montage only if we have a combat component and a weapon
	if (!Combat || !Combat->EquippedWeapon) return;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	
	if(AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage, Combat->EquippedWeapon->GetFireRate());
		FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AOnlineShooterCharacter::PlayHitReactMontage()
{
	// Play hit react montage only if we have a combat component and a weapon
	if (!Combat || !Combat->EquippedWeapon) return;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	
	if(AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AOnlineShooterCharacter::Multicast_Hit_Implementation()
{
	PlayHitReactMontage();
}

FVector AOnlineShooterCharacter::GetHitTarget() const
{
	if (!Combat) return FVector();

	return Combat->HitTarget;
}


