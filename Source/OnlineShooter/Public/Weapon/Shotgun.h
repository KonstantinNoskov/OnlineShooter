#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

UCLASS()
class ONLINESHOOTER_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()

public:
	
	AShotgun();
	virtual void Tick(float DeltaTime) override;

protected:
	
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	uint32 NumberOfPellets = 10; 

public:
	
	virtual void Fire(const FVector& HitTarget) override;
};
