#pragma once

#include "Components/CombatComponent.h"

// References
#include "Characters/OnlineShooterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"
#include "PlayerController/OnlineShooterPlayerController.h"
#include "Camera/CameraComponent.h"

// Kismet
#include "Kismet/GameplayStatics.h"

// Debug
#include "DrawDebugHelpers.h"

// HUD
#include "GameFramework/SpringArmComponent.h"
#include "HUD/OnlineShooterHUD.h"


// Constructor
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 800.f;
	AimingWalkSpeed = 400.f;
}

// Begin play
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
	}
}

// Tick
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshair(HitResult);
		HitTarget = HitResult.ImpactPoint;
		
		SetHUDCrosshair(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

// Replication
void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

// Aim
void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character && EquippedWeapon)
	{
		bAiming = bIsAiming;
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimingWalkSpeed : BaseWalkSpeed;

		Server_SetAiming(bIsAiming);
	}
}
void UCombatComponent::Server_SetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimingWalkSpeed : BaseWalkSpeed;
}

// Equip weapon
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (!Character || !WeaponToEquip) return;
	
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	
	// if had weapon socket...
	const USkeletalMeshSocket* WeaponHandSocket = Character->GetMesh()->GetSocketByName(FName("SKT_Weapon"));
	if (WeaponHandSocket)
	{
		// add equip weapon to a socket
		WeaponHandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}

	// Equipped weapon become owned by player that has picked it
	EquippedWeapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
	Character->StartingAimRotation = FRotator(0.f, Character->GetBaseAimRotation().Yaw, 0.f);
	
}
void UCombatComponent::OnRep_EquippedWeapon()
{
	if(EquippedWeapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;	
	}
}

// Fire 
void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFIreButtonPressed = bPressed;
	
	if (Character && bFIreButtonPressed)
	{
		FHitResult HitResult;
		TraceUnderCrosshair(HitResult);
		
		Server_Fire(HitResult.ImpactPoint);
	}
}
void UCombatComponent::Server_Fire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	Multicast_Fire(TraceHitTarget);
}
void UCombatComponent::Multicast_Fire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(!EquippedWeapon) return;
	
	if (Character && !EquippedWeapon->GetWeaponMesh()->IsPlaying())
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);

		CrosshairShootFactor = .75f;
	}
} 

// Crosshair & Aiming
void UCombatComponent::TraceUnderCrosshair(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Crosshair 2D position on viewport
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	// Storage for Crosshair 3D position and direction
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Result of crosshair deprojection
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);
	
	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
			
		}
		
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		// Shoot crosshair line trace
		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECC_Visibility);

		// if line trace didn't hit anything then impact point would be equal to TRACE_LENGTH
		if(!TraceHitResult.bBlockingHit) 
		{
			TraceHitResult.ImpactPoint = End;
		}
		else // otherwise show hit point
		{
			DrawDebugSphere(GetWorld(),TraceHitResult.ImpactPoint, 12.f, 12, FColor::Red);
		}

		// if line trace hit another player we want to change crosshair color
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
		}
		
	}
}
void UCombatComponent::SetHUDCrosshair(float DeltaTime)
{
	if(!Character || !Character->Controller) return;

	Controller = !Controller ? Cast<AOnlineShooterPlayerController>(Character->Controller) : Controller;

	if(Controller)
	{
		HUD = !HUD ? Cast<AOnlineShooterHUD>(Controller->GetHUD()) : HUD;

		if(HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairCenter	= EquippedWeapon->CrosshairCenter;
				HUDPackage.CrosshairLeft	= EquippedWeapon->CrosshairLeft;
				HUDPackage.CrosshairRight	= EquippedWeapon->CrosshairRight;
				HUDPackage.CrosshairTop	= EquippedWeapon->CrosshairTop;
				HUDPackage.CrosshairBottom	= EquippedWeapon->CrosshairBottom;
			}
			else
			{
				HUDPackage.CrosshairCenter	= nullptr;
				HUDPackage.CrosshairLeft	= nullptr;
				HUDPackage.CrosshairRight	= nullptr;
				HUDPackage.CrosshairTop	= nullptr;
				HUDPackage.CrosshairBottom	= nullptr;
			}
			
			// Calculate crosshair spread
			FVector2d WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			
			FVector2d VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f; 

			// Velocity Factor
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			// InAir Factor
			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			// Aim Factor
			if(bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, .58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			// Shoot Factor
			CrosshairShootFactor = FMath::FInterpTo(CrosshairShootFactor, 0.f, DeltaTime, 5.f);
			
			HUDPackage.CrosshairSpread =
				.5f +
				CrosshairVelocityFactor +
				CrosshairInAirFactor -
				CrosshairAimFactor +
				CrosshairShootFactor;
			
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if(!EquippedWeapon) return;

	if(bAiming)
	{	
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	if (Character && Character->GetFollowCamera())
	{
		 Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}





