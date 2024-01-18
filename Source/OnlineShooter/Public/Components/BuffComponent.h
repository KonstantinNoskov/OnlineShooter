#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


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

protected:

	UFUNCTION()
	void HealRampUp(float DeltaTime);
	
public:

	UFUNCTION()
	void Heal(float HealAmount, float HealingTime);
	
};


	
	
