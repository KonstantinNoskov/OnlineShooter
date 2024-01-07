#pragma once

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
	
	UFUNCTION(Server, Reliable)
	void Server_SetAiming(bool bIsAiming);

	UFUNCTION()
	void EquipWeapon(AWeapon* WeaponToEquip);

	UFUNCTION()
	void OnRep_EquippedWeapon();
	
	UFUNCTION()
	void TraceUnderCrosshair(FHitResult& TraceHitResult);

	UFUNCTION()
	void SetHUDCrosshair(float DeltaTime);

#pragma region FIRE

public:
	
	UFUNCTION()
	void FireButtonPressed(bool bPressed);
	
private:
	
	UPROPERTY()
	bool bFireButtonPressed;

	UPROPERTY(Replicated)
	float LastShotTime;

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
	
	// Ammo for the currently-equipped weapon carried by player
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UPROPERTY()
	TMap<EWeaponType, int32> CarriedAmmoMap;
	
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
	void OnFireTimerFinished();

	UFUNCTION()
	void OnRep_CarriedAmmo();

	UFUNCTION()
	void OnRep_CombatState();

#pragma endregion

#pragma region RELOAD

public:

	UFUNCTION()
	void Reload();
	
	UFUNCTION(Server, Reliable)
	void Server_Reload();

	UFUNCTION()
	void HandleReload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

protected:

	UFUNCTION()
	int32 AmountToReload();

private:

	UFUNCTION()
	void UpdateAmmoValues();
	
#pragma endregion

private:

	UPROPERTY()
	AOnlineShooterCharacter* Character;

	UPROPERTY()
	AOnlineShooterPlayerController* Controller;
	
	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;
	
	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Walk", meta = (AllowPrivateAccess = "true"))
	float BaseWalkSpeed;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aim", meta = (AllowPrivateAccess = "true"))
	float AimingWalkSpeed;

#pragma region HUD & CROSSHAIR

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

#pragma endregion

	FVector HitTarget;

#pragma region AIM & FOV

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

public:
	
	UFUNCTION()
	void SetAiming(bool bIsAiming);

#pragma endregion
	
};
