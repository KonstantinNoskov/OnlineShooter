#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


class UNiagaraComponent;
class AOnlineShooterCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONLINESHOOTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UBuffComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Friend Classes
	friend AOnlineShooterCharacter;
	
protected:
	
	virtual void BeginPlay() override;

private:

	UPROPERTY()
	AOnlineShooterCharacter* Character;

	UPROPERTY()
	bool bHealing = false;
	
	UPROPERTY()
	float HealingRate = 0.f;
	
	UPROPERTY()
	float AmountToHeal = 0.f;

	UPROPERTY()
	bool bReplenishShield = false;
	
	UPROPERTY()
	float ShieldReplenishAmount;
	
	UPROPERTY()
	float ShieldReplenishRate = 0.f;
	
	UPROPERTY()
	float InitialBaseSpeed;

	UPROPERTY()
	float InitialCrouchSpeed;

	UPROPERTY()
	float InitialJumpVelocity;
	
	UPROPERTY()
	FTimerHandle SpeedBuffTimer;

	UPROPERTY()
	FTimerHandle JumpBuffTimer;

	UPROPERTY()
	float BuffTime;

	UPROPERTY()
	UNiagaraComponent* BuffEffect;

private:

	UFUNCTION()
	void ResetSpeeds();

	UFUNCTION()
	void ResetJump();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateSpeeds(float BaseSpeed, float CrouchSpeed);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateJumpVelocity(float NewJumpVelocity);

protected:

	UFUNCTION()
	void HealRampUp(float DeltaTime);

	UFUNCTION()
	void ShieldRampUp(float DeltaTime);
	
public:

	UFUNCTION()
	void Heal(float HealAmount, float HealingTime);

	UFUNCTION()
	void ReplenishShield(float ShieldAmount, float ReplenishTime);

	UFUNCTION()
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float Time);

	UFUNCTION()
	void BuffJump(float BuffJumpVelocity, float Time);

	UFUNCTION()
	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);

	UFUNCTION()
	void SetInitialJumpVelocity(float Velocity);

	FORCEINLINE void SetBuffEffect(UNiagaraComponent* NewEffect) { BuffEffect = NewEffect; }
	FORCEINLINE float GetShieldReplenishAmount() const { return ShieldReplenishAmount; }
	
};


	
	
