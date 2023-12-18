// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OnlineShooter/Data/TurningInPlace.h"

// INPUT
#include "InputActionValue.h"
#include "Components/CombatComponent.h"

// INTERFACES
#include "Interfaces/InteractInterface.h"

#include "OnlineShooterCharacter.generated.h"


class UCombatComponent;
class AWeapon;
class UWidgetComponent;
class USpringArmComponent;
class UCameraComponent;

// INPUT
class UInputMappingContext;
class UInputAction;
struct FInputActionInstance;

UCLASS()
class ONLINESHOOTER_API AOnlineShooterCharacter : public ACharacter, public IInteractInterface
{
	GENERATED_BODY()

public:

	// Constructors
	AOnlineShooterCharacter();

	// Basic overrides
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void OnRep_ReplicatedMovement() override;

	// Friend Classes
	friend UCombatComponent;

protected:
	
	virtual void BeginPlay() override;

	virtual void Jump() override;

private:

#pragma region COMPONENTS
	
	// Camera boom positioning the camera behind the character 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	// Follow camera 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	// Follow camera 
	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PlayerWidget, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;*/

	// Combat component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* Combat;

#pragma endregion

#pragma region INPUT
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* CharacterMappingContext;
	
	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Crouch Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;
	
	/** Equip Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipAction;

	/** Aim Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

protected:
	
	/** Callback for movement input */
	void Move(const FInputActionValue& Value);

	/** Callback for looking input */
	void Look(const FInputActionValue& Value);

	/** Callback for Equip input */
	void EquipButtonPressed();

	/** Callback for Crouch input */
	void CrouchButtonPressed();
	
	/** Callback for Crouch input */
	void CrouchButtonReleased();

	/** Callback for Aim input */
	void AimButtonPressed();
	void AimButtonReleased();
	void CalculateAO_Pitch();

	/** Callback for Fire input */
	void FireButtonPressed(const FInputActionInstance& Instance);
	void FireButtonReleased(const FInputActionInstance& Instance);

private:

	// Servers Inputs
	UFUNCTION(Server, Reliable)
	void Server_EquipButtonPressed();

#pragma endregion

private:

	UPROPERTY()
	float AO_Yaw;

	UPROPERTY()
	float AO_Pitch;
	
	UPROPERTY()
	float InterpAO_Yaw;

	UPROPERTY()
	ETurningInPlace TurningInPlace;

	UPROPERTY(ReplicatedUsing = OnRep_OverllapingWeapon)
	AWeapon* OverlappingWeapon;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY()
	bool bRotateRootBone;

	UPROPERTY()
	float TurnThreshold = .5f;

	UPROPERTY()
	FRotator ProxyRotationLastFrame;

	UPROPERTY()
	FRotator ProxyRotation;

	UPROPERTY()
	float ProxyYaw;

	UPROPERTY()
	float TimeSinceLastMovementReplication;

	

public:
	
	UPROPERTY(EditAnywhere, Category = "WeaponRotationCorrection") 
	float RightHandRotationRoll;
	
	UPROPERTY(EditAnywhere, Category = "WeaponRotationCorrection") 
	float RightHandRotationYaw;
	
	UPROPERTY(EditAnywhere, Category = "WeaponRotationCorrection") 
	float RightHandRotationPitch;

private:
	
	UFUNCTION()
	void AimOffset(float DeltaTime);

	UFUNCTION()
	void SimProxiesTurn();
	
	UFUNCTION() // Handles character turn in place animations depending on Z-axis offset.
	void TurnInPlace(float DeltaTime);
	
	UFUNCTION() 
	void OnRep_OverllapingWeapon(AWeapon* LastWeapon);
	
	UFUNCTION() // Hides meshes when camera too close
	void HideMesh();

public:
	// Determines whether player overlap weapon
	void SetOverlappingWeapon(AWeapon* Weapon);

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsWeaponEquipped() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAiming() const;
	
	FRotator StartingAimRotation;
	
	AWeapon* GetEquippedWeapon() const;
	
	UFUNCTION()
	void PlayFireMontage(bool bAiming);

	UFUNCTION()
	void PlayHitReactMontage();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_Hit();

	FVector GetHitTarget() const;
	
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
};
