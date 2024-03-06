

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

protected:

	virtual void BeginPlay() override;

public:
	
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void OnBounceGrenade(const FHitResult& ImpactResult, const FVector& ImpactVelocity) override;

public:
	virtual void Destroyed() override;
};
