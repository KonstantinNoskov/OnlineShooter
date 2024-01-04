#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class USoundCue;
class UProjectileMovementComponent;
class UBoxComponent;

UCLASS()
class ONLINESHOOTER_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	
	UPROPERTY(EditAnywhere, Category = "Projectile Properties")
	UParticleSystem* Tracer;
	
	UPROPERTY()
	UParticleSystemComponent* TracerComponent;

protected:
	
	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComponent;
	
	UPROPERTY(EditAnywhere) 
	UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere, Category = "Projectile Properties")
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, Category = "Projectile Properties")
	USoundCue* ImpactSound;
	
	UPROPERTY(EditAnywhere, Category = "Projectile Properties")
	float Damage = 10.f;
};
