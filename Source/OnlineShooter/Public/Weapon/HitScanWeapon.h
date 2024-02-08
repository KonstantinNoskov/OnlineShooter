#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

UCLASS()
class ONLINESHOOTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	
	virtual void Tick(float DeltaTime) override;
	virtual void Fire(const FVector& HitTarget) override;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

private:
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;

protected:
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	USoundCue* ImpactSound;
	
public:
	
	
	
protected:
	
	
};
