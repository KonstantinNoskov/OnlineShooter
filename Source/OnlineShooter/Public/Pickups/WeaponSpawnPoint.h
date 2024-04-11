#pragma once

#include "CoreMinimal.h"
#include "PickupSpawnPoint.h"
#include "WeaponSpawnPoint.generated.h"

UCLASS()
class ONLINESHOOTER_API AWeaponSpawnPoint : public APickupSpawnPoint
{
	GENERATED_BODY()

public:
	
	AWeaponSpawnPoint();

protected:
	virtual void SpawnPickup() override;

public:
	virtual void SetSpawnPointHighLight(bool bPickedUp) override;

private:
	UPROPERTY(EditAnywhere, Category = "Weapon Spawn Point")
	TArray<TSubclassOf<AWeapon>> WeaponClasses;

	UPROPERTY()
	AWeapon* SpawnedWeapon;
};
