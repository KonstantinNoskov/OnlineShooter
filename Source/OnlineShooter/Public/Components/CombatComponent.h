#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

// References
class AWeapon;
class AOnlineShooterCharacter;

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

private:

	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY()
	AOnlineShooterCharacter* Character;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Walk", meta = (AllowPrivateAccess = "true"))
	float BaseWalkSpeed;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aim", meta = (AllowPrivateAccess = "true"))
	float AimingWalkSpeed;

	UFUNCTION()
	void EquipWeapon(AWeapon* WeaponToEquip);

	UFUNCTION()
	void OnRep_EquippedWeapon();
	
};
