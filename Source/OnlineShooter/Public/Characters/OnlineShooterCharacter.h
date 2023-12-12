// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OnlineShooter/Data/TurningInPlace.h"

// INPUT
#include "InputActionValue.h"
#include "Components/CombatComponent.h"

#include "OnlineShooterCharacter.generated.h"

class UCombatComponent;
class AWeapon;
class UWidgetComponent;
class USpringArmComponent;
class UCameraComponent;

class UInputMappingContext;
class UInputAction;

UCLASS()
class ONLINESHOOTER_API AOnlineShooterCharacter : public ACharacter
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

	/** Callback for Fire input */
	void FireButtonPressed();
	void FireButtonReleased();
	

private:

	// Servers Inputs
	UFUNCTION(Server, Reliable)
	void Server_EquipButtonPressed();

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* FireWeaponMontage;


#pragma endregion

private:

	void AimOffset(float DeltaTime);

	float AO_Yaw;
	float AO_Pitch;
	float InterpAO_Yaw;
	
	ETurningInPlace TurningInPlace;

	UFUNCTION()
	void TurnInPlace(float DeltaTime);
	
	UPROPERTY(ReplicatedUsing = OnRep_OverllapingWeapon)
	AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverllapingWeapon(AWeapon* LastWeapon);

public:
	// Determines whether player overlap weapon
	void SetOverlappingWeapon(AWeapon* Weapon);

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsWeaponEquipped() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAiming() const;

	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FRotator StartingAimRotation;
	
	AWeapon* GetEquippedWeapon() const;
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }

	UFUNCTION()
	void PlayFireMontage(bool bAiming);
	
};
