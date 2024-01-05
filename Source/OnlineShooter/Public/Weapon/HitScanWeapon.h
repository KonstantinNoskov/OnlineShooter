#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

UCLASS()
class ONLINESHOOTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	
	AHitScanWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void Fire(const FVector& HitTarget) override;

protected:
	
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere)
	float Damage = 5.f;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

public:
	
	
};
