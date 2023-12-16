﻿#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

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

	void FireButtonPressed(bool bPressed);

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
	
	bool bFIreButtonPressed;
	

#pragma region HUD & CROSSHAIR

	UPROPERTY()
	AOnlineShooterHUD* HUD;

	UPROPERTY()
	float CrosshairVelocityFactor;

	UPROPERTY()
	float CrosshairInAirFactor;

#pragma endregion

	FVector HitTarget;
	
};
