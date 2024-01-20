#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "ShieldPickup.generated.h"

UCLASS()
class ONLINESHOOTER_API AShieldPickup : public APickup
{
	GENERATED_BODY()

public:
	AShieldPickup();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	
	UPROPERTY(EditAnywhere)
	float ShieldReplenishAmount = 30.f;

	UPROPERTY(EditAnywhere)
	float ShieldReplenishTime  = 5.f;
	
};
