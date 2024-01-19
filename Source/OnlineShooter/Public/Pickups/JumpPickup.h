#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "JumpPickup.generated.h"

UCLASS()
class ONLINESHOOTER_API AJumpPickup : public APickup
{
	GENERATED_BODY()

public:
	AJumpPickup();

protected:
	virtual void BeginPlay() override;
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

public:
	virtual void Tick(float DeltaTime) override;

private:

	UPROPERTY(EditAnywhere)
	float JumpZVelocityBuff = 4000.f;

	UPROPERTY(EditAnywhere)
	float JumpBuffTime = 15.f;
};
