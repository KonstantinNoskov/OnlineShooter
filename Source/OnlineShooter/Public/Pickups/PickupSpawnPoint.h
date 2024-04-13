#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

class AWeapon;
class APickup;

UCLASS()
class ONLINESHOOTER_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

private:

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* SpawnPointMesh;
	
	UPROPERTY(EditAnywhere, Category = "Spawn Time")
	float SpawnPickupTimeMin;

	UPROPERTY(EditAnywhere, Category = "Spawn Time")
	float SpawnPickupTimeMax;
	
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<APickup>> PickupClasses;
	
	UPROPERTY()
	APickup* SpawnedPickup;

protected:

	UPROPERTY()
	FTimerHandle SpawnPickupTimer;
	
	UPROPERTY(EditAnywhere)
	USoundBase* SpawnSound;
	
	UPROPERTY(EditAnywhere, Category = "Spawn Time")
	bool bSpawnLoop = true;
	
	UPROPERTY(EditAnywhere, Category = "Spawn Time", meta = (EditCondition = "!bSpawnLoop"))
	int32 SpawnRepeatAmount = 0;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInstance* SpawnOnMaterial;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInstance* SpawnOffMaterial;

protected:

	UFUNCTION()
	virtual void SpawnPickup();

	UFUNCTION()
	void SpawnPickupFinished();

public:

	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);
	
	virtual void SetSpawnPointHighLight(bool bPickedUp);
};
