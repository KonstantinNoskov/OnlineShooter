#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UBoxComponent;

UCLASS()
class ONLINESHOOTER_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	AProjectile();

protected:
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox;

public:
	virtual void Tick(float DeltaTime) override;
};
