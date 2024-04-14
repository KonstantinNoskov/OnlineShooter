#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

class APickupSpawnPoint;
class AOnlineShooterCharacter;
class UNiagaraSystem;
class UNiagaraComponent;
class USoundCue;
class USphereComponent;

UCLASS()
class ONLINESHOOTER_API APickup : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	USphereComponent* OverlapSphere;

public:
	APickup();

	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UPROPERTY()
	APickupSpawnPoint* SpawnPointOwner;

public:
	FORCEINLINE void SetSpawnPointOwner(APickupSpawnPoint* NewOwner) { SpawnPointOwner = NewOwner; }

private:
	
	UPROPERTY(EditAnywhere)
	USoundCue* PickupSound;
	
	UPROPERTY(EditAnywhere)
	UNiagaraSystem* PickupEffect;

	UPROPERTY()
	FTimerHandle BindOverlapTimer;

	UPROPERTY()
	float BindOverlapTime = .25f;

protected:

	/* If greater than 0 overrides inherited pickup's values */
	UPROPERTY()
	float SpawnedPickupValue = 0.f;

	UPROPERTY(VisibleAnywhere)
	UNiagaraComponent* PickupEffectComponent;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickupMesh;
	
	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayEffect(AOnlineShooterCharacter* OverlappedCharacter);
	
private:
	
	UFUNCTION()
	void BindOverlapTimerFinished();

public:

	FORCEINLINE void SetSpawnedPickupValue(float NewValue) { SpawnedPickupValue = NewValue; }
	
};
