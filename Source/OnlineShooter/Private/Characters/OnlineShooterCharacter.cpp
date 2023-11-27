#include "Characters/OnlineShooterCharacter.h"

// INPUT
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

// Replication
#include "Net/UnrealNetwork.h"

// References
#include "Weapon/Weapon.h"

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

	// Create Overhead Widget
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	// Create Combat Component
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	// Collision preset
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
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


