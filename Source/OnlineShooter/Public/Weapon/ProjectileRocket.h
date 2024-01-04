#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

class URocketMovementComponent;
class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class ONLINESHOOTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	
	AProjectileRocket();

	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

protected:
	
	virtual void BeginPlay() override;

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit) override;

	UPROPERTY(EditAnywhere, Category = "Rocket")
	float MinDamage = 10.f;

	UPROPERTY(EditAnywhere, Category = "Rocket")
	float InnerRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "Rocket")
	float OuterRadius = 500.f;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* TrailSystem;

	UPROPERTY()
	UNiagaraComponent* TrailSystemComponent;

	UPROPERTY(EditAnywhere)
	USoundCue* ProjectileLoop;

	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;

	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;
	
private:
	
	UPROPERTY(VisibleAnywhere)
	URocketMovementComponent* RocketMovementComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;

	UPROPERTY()
	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;
	
protected:

	UFUNCTION()
	void DestroyTimerFinished();
	
	
};
