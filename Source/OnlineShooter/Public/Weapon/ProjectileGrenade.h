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
	
	void OnInitialSpeedEdit(const FPropertyChangedEvent& PropertyChangedEventGrenade);
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnBounceGrenade(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	UPROPERTY(EditAnywhere)
	USoundCue* BounceSound;
		
private:
	
	UFUNCTION()
	void OnProjectileGrenadeStop(const FHitResult& ImpactResult);
};

