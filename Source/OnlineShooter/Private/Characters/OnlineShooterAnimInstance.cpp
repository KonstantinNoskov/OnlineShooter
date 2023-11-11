// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/OnlineShooterAnimInstance.h"

#include "Characters/OnlineShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UOnlineShooterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Get the reference to a player character 
	OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(TryGetPawnOwner());
	
}

void UOnlineShooterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	/* ANIMATION PROPERTIES
	 * ==============================================================================================================
	 * 
	 * Following properties will determine which character animation is gonna be played
	 * Properties updates every "animation tick"
	 *
	 * @var Speed - horizontal speed
	 * @var bIsInAir - whether character is falling, jumping, flying etc.
	 * @var bIsAccelerating - it's NOT the rate of change of velocity in physical understanding. It's just determines
	 *		whether player pressed keyboard key to move.
	 *		
	 *
	 * ==============================================================================================================
	 */
	
	// if player reference is nov valid, try to get player reference
	if (!OnlineShooterCharacter) { OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(TryGetPawnOwner()); }

	// if player reference still is null, return
	if(!OnlineShooterCharacter) { return; }

	// Speed (horizontal)
	FVector Velocity = OnlineShooterCharacter->GetVelocity();
	Speed = Velocity.Size2D();

	bIsInAir = OnlineShooterCharacter->GetCharacterMovement()->IsFalling();
	
	bIsAccelerating = OnlineShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;
}
