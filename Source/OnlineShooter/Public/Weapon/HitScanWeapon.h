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

	UFUNCTION()
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);

	UFUNCTION()
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

private:
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;

protected:

	UPROPERTY(EditAnywhere)
	float Damage = 5.f;
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	USoundCue* ImpactSound;
	

public:
	
	
};
