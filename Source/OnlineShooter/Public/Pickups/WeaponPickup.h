#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "WeaponPickup.generated.h"

UCLASS()
class ONLINESHOOTER_API AWeaponPickup : public APickup
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWeaponPickup();

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};
