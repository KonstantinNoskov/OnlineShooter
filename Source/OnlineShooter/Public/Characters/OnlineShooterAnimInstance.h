// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "OnlineShooter/Data/TurningInPlace.h"
#include "OnlineShooterAnimInstance.generated.h"


class AOnlineShooterCharacter;
class AWeapon;

/**
 * 
 */
UCLASS()
class ONLINESHOOTER_API UOnlineShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:

	/* ANIMATION PROPERTIES
	 * ==============================================================================================================
	 * 
	 * Following properties will determine which of character's animation is gonna be played
	 * The properties updates every "animation tick"
	 *
	 * @var OnlineShooterCharacter	- Reference to character
	 * @var Speed					- horizontal speed
	 * @var bIsInAir				- whether character is falling, jumping, flying etc.
	 * @var bIsAccelerating			- it's NOT the rate of change of velocity in physical understanding. It's just determines
	 *								  whether player pressed keyboard key to move.
	 * @var bWeaponEquipped			- checks if player has a equipped weapon
	 * @var bIsCrouching			- character crouching
	 * @var bAiming					- character aiming
	 * @var YawOffset				- 
	 * @var Lean					-
	 * ==============================================================================================================
	 */

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	AOnlineShooterCharacter* OnlineShooterCharacter;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bWeaponEquipped;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsCrouching;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float YawOffset;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float Lean;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	ETurningInPlace TurningInPlace;

	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator StrafingDeltaRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Aim Offset", meta = (AllowPrivateAccess = "true"))
	float AO_Yaw;

	UPROPERTY(BlueprintReadOnly, Category = "Aim Offset", meta = (AllowPrivateAccess = "true"))
	float AO_Pitch;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon Grip", meta = (AllowPrivateAccess = "true"))
	FTransform WeaponLeftHandTransform;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon Grip", meta = (AllowPrivateAccess = "true"))
	FRotator RightHandRotation;
	

	UPROPERTY(BlueprintReadOnly, Category = "Weapon Grip", meta = (AllowPrivateAccess = "true"))
	bool bLocallyControlled;

	
};
