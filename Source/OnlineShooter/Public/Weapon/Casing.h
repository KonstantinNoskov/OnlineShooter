#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

class USoundCue;

UCLASS()
class ONLINESHOOTER_API ACasing : public AActor
{
	GENERATED_BODY()

public:
	
	ACasing();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

protected:
	
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Casing Properties")
	float ShellEjectionImpulse;

	UPROPERTY(EditAnywhere, Category = "Casing Properties")
	USoundCue* ShellSound;

private:

	UPROPERTY(EditAnywhere, Category = "Casing Properties" )
	UStaticMeshComponent* CasingMesh;

	UPROPERTY(EditAnywhere, Category = "Casing Properties")
	float CasingSpinYaw;
	
	UPROPERTY(EditAnywhere, Category = "Casing Properties")
	float CasingSpinPitch;
	
	UPROPERTY(EditAnywhere, Category = "Casing Properties")
	float CasingSpinRoll;

	UPROPERTY(EditAnywhere, Category = "Casing Properties")
	float CasingLifeTime;
	
	
};
