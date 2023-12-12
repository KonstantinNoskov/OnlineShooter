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
	float EjectionRightImpulseMin;
	
	UPROPERTY(EditAnywhere, Category = "Casing Properties")
	float EjectionRightImpulseMax;

	UPROPERTY(EditAnywhere, Category = "Casing Properties")
	float EjectionUpImpulseMin;
	
	UPROPERTY(EditAnywhere, Category = "Casing Properties")
	float EjectionUpImpulseMax;

	UPROPERTY(EditAnywhere, Category = "Casing Properties")
	float EjectionFwdImpulseMin;
	
	UPROPERTY(EditAnywhere, Category = "Casing Properties")
	float EjectionFwdImpulseMax;

	UPROPERTY(EditAnywhere, Category = "Casing Properties")
	USoundCue* ShellSound;

private:

	UPROPERTY(EditAnywhere, Category = "Casing Properties" )
	UStaticMeshComponent* CasingMesh;

	UPROPERTY(EditAnywhere, Category = "Casing Properties")
	float CasingSpinYawMin;
	
	UPROPERTY(EditAnywhere, Category = "Casing Properties")
	float CasingSpinYawMax;

	UPROPERTY(EditAnywhere, Category = "Casing Properties")
	float CasingSpinPitchMin;
	
	UPROPERTY(EditAnywhere, Category = "Casing Properties")
	float CasingSpinPitchMax;

	UPROPERTY(EditAnywhere, Category = "Casing Properties")
	float CasingSpinRollMin;
	
	UPROPERTY(EditAnywhere, Category = "Casing Properties")
	float CasingSpinRollMax;

	UPROPERTY(EditAnywhere, Category = "Casing Properties")
	float CasingLifeTime;
	
	
};
