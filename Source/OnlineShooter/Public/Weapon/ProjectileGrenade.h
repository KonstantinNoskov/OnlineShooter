#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileGrenade.generated.h"

class URocketMovementComponent; 

UCLASS()
class ONLINESHOOTER_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()

public:
	
	AProjectileGrenade();
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;

protected:
	
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);
		
private:

	UPROPERTY(VisibleAnywhere)
	URocketMovementComponent* RocketMovementComponent;
	
	UPROPERTY(EditAnywhere)
	USoundCue* BounceSound;
};
