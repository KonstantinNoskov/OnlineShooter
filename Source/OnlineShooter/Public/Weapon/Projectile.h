#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class USoundCue;
class UProjectileMovementComponent;
class UBoxComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class AOnlineShooterCharacter;

UCLASS()
class ONLINESHOOTER_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

	bool bUserServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000.f;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
private:

	UPROPERTY(EditAnywhere, Category = "Projectile")
	bool bDebug = false;
	
	UPROPERTY(EditAnywhere, Category = "Projectile")
	UParticleSystem* Tracer;
	
	UPROPERTY()
	UParticleSystemComponent* TracerComponent;

	UPROPERTY()
	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float DestroyTime = 3.f;

protected:

	UPROPERTY(EditAnywhere, Category = "Explosion Damage")
	float ExplosionMinDamage = 10.f;

	UPROPERTY(EditAnywhere, Category = "Explosion Damage")
	float ExplosionInnerRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "Explosion Damage")
	float ExplosionOuterRadius = 500.f;
	
	UPROPERTY(EditAnywhere)
	UProjectileMovementComponent* ProjectileMovementComponent;
	
	UPROPERTY(EditAnywhere) 
	UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	USoundCue* ImpactSound;
	
	UPROPERTY(EditAnywhere, Category = "Projectile")
	UNiagaraSystem* TrailSystem;

	UPROPERTY()
	UNiagaraComponent* TrailSystemComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;


public:
	
	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	float Damage = 10.f;
	
protected:

	UFUNCTION()
	void StartDestroyTimer();
	
	UFUNCTION()
	void DestroyTimerFinished();

	UFUNCTION()
	void SpawnTrailSystem();

	UFUNCTION()
	void ExplodeDamage();
	
};
