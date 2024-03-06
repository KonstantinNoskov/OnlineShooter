

#pragma once

#include "CoreMinimal.h"
#include "ProjectileGrenade.h"
#include "ThrowGrenade.generated.h"

UCLASS()
class ONLINESHOOTER_API AThrowGrenade : public AProjectileGrenade
{
	GENERATED_BODY()
	
public:

	AThrowGrenade();
	virtual void Tick(float DeltaTime) override;

protected:

	virtual void BeginPlay() override;
	virtual void OnBounceGrenade(const FHitResult& ImpactResult, const FVector& ImpactVelocity) override;

public:
	virtual void Destroyed() override;

private:
	virtual void OnProjectileGrenadeStop(const FHitResult& ImpactResult) override;

	UPROPERTY(EditAnywhere, Category = "Throw Grenade")
	UParticleSystemComponent* ThrowGrenadeParticleComponent;
	
};
