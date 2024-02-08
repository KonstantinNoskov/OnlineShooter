// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/OnlineShooterAnimInstance.h"

// Math
#include "Kismet/KismetMathLibrary.h"

// References
#include "Characters/OnlineShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapon/Weapon.h"

void UOnlineShooterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Get the reference to a player character 
	OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(TryGetPawnOwner());
}

void UOnlineShooterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	// if player reference is not valid, try to get player reference
	if (!OnlineShooterCharacter) { OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(TryGetPawnOwner()); }

	// if player reference still is null, return
	if(!OnlineShooterCharacter) { return; }

	// Speed (horizontal)
	Speed = OnlineShooterCharacter->GetVelocity().Size2D();

	// Is character eliminated
	bEliminated = OnlineShooterCharacter->IsEliminated();

	// Is character jumping
	bIsInAir = OnlineShooterCharacter->GetCharacterMovement()->IsFalling();

	// Is character moving
	bIsAccelerating = OnlineShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;

	// Does character have a equipped weapon
	bWeaponEquipped = OnlineShooterCharacter->IsWeaponEquipped();

	// Returns character's equipped weapon. 
	EquippedWeapon = OnlineShooterCharacter->GetEquippedWeapon();

	// Is character crouching
	bIsCrouching = OnlineShooterCharacter->bIsCrouched;

	// Is character aiming
	bAiming = OnlineShooterCharacter->IsAiming();

	// Which side the character is turning to
	TurningInPlace = OnlineShooterCharacter->GetTurningInPlace();
	
	bRotateRootBone = OnlineShooterCharacter->ShouldRotateRootBone();
	
	// Calculate Offset Yaw for Strafing
	FRotator AimRotation = OnlineShooterCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(OnlineShooterCharacter->GetVelocity());
	
	FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	StrafingDeltaRotation = FMath::RInterpTo(StrafingDeltaRotation, Delta, DeltaSeconds, 6.f);
	YawOffset = StrafingDeltaRotation.Yaw;
	
	// Calculate Lean angle
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = OnlineShooterCharacter->GetActorRotation();
	const FRotator CharacterRotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = CharacterRotationDelta.Yaw / DeltaSeconds;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	// Calculate AimOffset
	AO_Yaw = OnlineShooterCharacter->GetAO_Yaw();
	AO_Pitch = OnlineShooterCharacter->GetAO_Pitch();

	// Calculate weapon socket transform
	if(bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh())
	{
		WeaponLeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("SKT_LeftHand"), RTS_World);
		FVector OutPositionLeft;
		FRotator OutRotationLeft;
		OnlineShooterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), WeaponLeftHandTransform.GetLocation(), FRotator::ZeroRotator,
			OutPositionLeft, OutRotationLeft);

		WeaponLeftHandTransform.SetLocation(OutPositionLeft);
		WeaponLeftHandTransform.SetRotation(FQuat(OutRotationLeft));
		
		if (OnlineShooterCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			
			FTransform RightHandTransform = OnlineShooterCharacter->GetMesh()->GetSocketTransform(FName("hand_r"), RTS_World);

			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(FVector(), RightHandTransform.GetLocation() - OnlineShooterCharacter->GetHitTarget());

			LookAtRotation.Roll += OnlineShooterCharacter->RightHandRotationRoll;
			LookAtRotation.Yaw += OnlineShooterCharacter->RightHandRotationYaw;
			LookAtRotation.Pitch += OnlineShooterCharacter->RightHandRotationPitch;
			
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaSeconds, 20.f);// UKismetMathLibrary::FindLookAtRotation(FVector(), RightHandTransform.GetLocation() - OnlineShooterCharacter->GetHitTarget());
			
		}
		
		FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), RTS_World);
		FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));


		if (bDebug)
		{
			// Debug Lines for correcting muzzle rotation towards to crosshair
			DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f, FColor::Red);
			DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), OnlineShooterCharacter->GetHitTarget(), FColor::Yellow);	
		}
	}
	
	// Use FABRIK all the time except reloading. FABRIK used to always place left hand on weapon 
	bUseFABRIK = OnlineShooterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;

	bool bFABRIKOverride =
		OnlineShooterCharacter->IsLocallyControlled() &&
		OnlineShooterCharacter->GetCombatState() != ECombatState::ECS_ThrowingGrenade;

	// Stop using FABRIK if Reload starts on Client to prevent 
	if (bFABRIKOverride)
	{
		bUseFABRIK = !OnlineShooterCharacter->IsLocallyReloading();
	}
	
	// Use AimOffsets all the time except reloading & when the game state is on cooldown. 
	bUseAimOffsets = OnlineShooterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !OnlineShooterCharacter->GetDisableGameplay();

	// Use Right hand transform all the time except reloading & when the game state is on cooldown.
	// Right hand transform used to aim weapon mesh towards to hit target.
	bTransformRightHand = OnlineShooterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !OnlineShooterCharacter->GetDisableGameplay();
}
