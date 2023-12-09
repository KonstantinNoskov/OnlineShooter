#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

UCLASS()
class ONLINESHOOTER_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	AProjectileWeapon();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
