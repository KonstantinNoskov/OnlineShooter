﻿#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

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

private:
	UPROPERTY(EditAnywhere)
	USoundCue* PickupSound;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickupMesh;

protected:
	
	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;

public:
	
};
