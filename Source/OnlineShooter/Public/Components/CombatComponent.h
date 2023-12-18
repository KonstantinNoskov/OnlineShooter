#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/OnlineShooterHUD.h"
#include "CombatComponent.generated.h"

struct FInputActionInstance;
// References
class AWeapon;
class AOnlineShooterCharacter;
class AOnlineShooterPlayerController;
class AOnlineShooterHUD;

#define TRACE_LENGTH 80000.f

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

	UFUNCTION()
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void Server_SetAiming(bool bIsAiming);

	UFUNCTION()
	void EquipWeapon(AWeapon* WeaponToEquip);

	UFUNCTION()
	void OnRep_EquippedWeapon();
	void Fire();

	void FireButtonPressed(bool bPressed, FInputActionInstance InputActionInstance);

	UFUNCTION(Server, Reliable)
	void Server_Fire(const FVector_NetQuantize& TraceHitTarget);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Fire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION()
	void TraceUnderCrosshair(FHitResult& TraceHitResult);

	UFUNCTION()
	void SetHUDCrosshair(float DeltaTime);

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
	
	bool bFireButtonPressed;

	float LastShotTime;
	

#pragma region HUD & CROSSHAIR

	UPROPERTY()
	AOnlineShooterHUD* HUD;

	FHUDPackage HUDPackage;

	UPROPERTY()
	float CrosshairVelocityFactor;

	UPROPERTY()
	float CrosshairInAirFactor;

	UPROPERTY()
	float CrosshairAimFactor;

	UPROPERTY()
	float CrosshairShootFactor;

	UPROPERTY()
	FTimerHandle FireTimer;
	
	void StartFireTimer();
	void OnFireTimerFinished();
	
	bool bCanFire = true;

#pragma endregion

	FVector HitTarget;

	/*
	 * Aiming and FOV
	 */

	// Field of view when not aiming; set to the camera's base FOV in BeginPlay
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	float CurrentFOV;

	void InterpFOV(float DeltaTime);
	
};
