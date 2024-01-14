﻿#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Components/ActorComponent.h"
#include "HUD/OnlineShooterHUD.h"
#include "OnlineShooter/Data/CombatState.h"
#include "Weapon/WeaponTypes.h"
#include "CombatComponent.generated.h"


// References
class AWeapon;
class AOnlineShooterCharacter;
class AOnlineShooterPlayerController;
class AOnlineShooterHUD;
struct FInputActionInstance;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONLINESHOOTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UCombatComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// Friend Classes
	friend AOnlineShooterCharacter;

protected:
	
	virtual void BeginPlay() override;

private:
	
	UPROPERTY()
	bool bFireButtonPressed;
	
	UPROPERTY()
	FTimerHandle FireTimer;

	UPROPERTY()
	EWeaponType WeaponType;

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState;
	
	UPROPERTY()
	bool bCanFire = true;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 3;

	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 3;
	
	// Ammo for the currently-equipped weapon carried by player
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UPROPERTY()
	TMap<EWeaponType, int32> CarriedAmmoMap;
	
	UPROPERTY()
	AOnlineShooterCharacter* Character;

	UPROPERTY()
	AOnlineShooterPlayerController* Controller;
	
	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;
	
	UPROPERTY(Replicated)
	bool bAiming;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Walk", meta = (AllowPrivateAccess = "true"))
	float BaseWalkSpeed;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aim", meta = (AllowPrivateAccess = "true"))
	float AimingWalkSpeed;

	UPROPERTY()
	AOnlineShooterHUD* HUD;

	UPROPERTY()
	FHUDPackage HUDPackage;

	UPROPERTY()
	float CrosshairVelocityFactor;

	UPROPERTY()
	float CrosshairInAirFactor;

	UPROPERTY()
	float CrosshairAimFactor;

	UPROPERTY()
	float CrosshairShootFactor;

	FVector HitTarget;
	
	/*
	 * Aiming and FOV
	 */

	// Field of view when not aiming; set to the camera's base FOV in BeginPlay
	
	UPROPERTY()
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	UPROPERTY()
	float CurrentFOV;

	UFUNCTION()
	void InterpFOV(float DeltaTime);


private:

	UFUNCTION()
	void UpdateAmmoValues();

	UFUNCTION()
	void UpdateShotgunAmmoValues();

	UFUNCTION()
	void UpdateHUDGrenades();

	UFUNCTION()
	bool CanFire();

	UFUNCTION()
	void Fire();
	
	UFUNCTION(Server, Reliable)
	void Server_Fire(const FVector_NetQuantize& TraceHitTarget);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Fire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION()
	void StartFireTimer();
	
	UFUNCTION() 
	void InitializeCarriedAmmo();

	UFUNCTION()
	void ShowAttachedGrenade(bool bShowGrenade);

	UFUNCTION()
	void OnFireTimerFinished();

	UFUNCTION()
	void OnRep_CarriedAmmo();

	UFUNCTION()
	void OnRep_CombatState();

	UFUNCTION()
	void OnRep_Grenades();

protected:

	UFUNCTION()
	int32 AmountToReload();

	UFUNCTION(Server, Reliable)
	void Server_SetAiming(bool bIsAiming);
	
	UFUNCTION()
	void EquipWeapon(AWeapon* WeaponToEquip);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void DropEquippedWeapon();

	UFUNCTION()
	void AttachActorToRightHand(AActor* ActorToAttach);

	UFUNCTION()
	void AttachActorToLeftHand(AActor* ActorToAttach);

	UFUNCTION()
	void UpdateCarriedAmmo();

	UFUNCTION()
	void PlayEquippedWeaponSound();

	UFUNCTION()
	void ReloadEmptyWeapon();
	
	UFUNCTION()
	void TraceUnderCrosshair(FHitResult& TraceHitResult);

	UFUNCTION()
	void SetHUDCrosshair(float DeltaTime);

	UFUNCTION()
	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void Server_ThrowGrenade();

public:

	UFUNCTION()
	void FireButtonPressed(bool bPressed);

	UFUNCTION()
	void Reload();
	
	UFUNCTION(Server, Reliable)
	void Server_Reload();

	UFUNCTION()
	void HandleReload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION()
	void SetAiming(bool bIsAiming);

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	UFUNCTION()
	void JumpToShotgunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
	void Server_LaunchGrenade(const FVector_NetQuantize& Target);


	FORCEINLINE int32 GetGrenades() const { return Grenades; }
	
};
