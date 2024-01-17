﻿#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "Weapon/WeaponTypes.h"
#include "AmmoPickup.generated.h"


UCLASS()
class ONLINESHOOTER_API AAmmoPickup : public APickup
{
	GENERATED_BODY()

public:
	
	AAmmoPickup();
	virtual void Tick(float DeltaTime) override;

protected:
	
	virtual void BeginPlay() override;
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
		) override;
	
private:

	UPROPERTY(EditAnywhere)
	int32 AmmoAmount = 30;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
	
};
