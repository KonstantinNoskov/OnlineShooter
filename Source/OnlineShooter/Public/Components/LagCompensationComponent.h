#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

class AOnlineShooterPlayerController;

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONLINESHOOTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULagCompensationComponent();

	friend class AOnlineShooterCharacter;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	virtual void BeginPlay() override;
	void SaveFrameHistory(float DeltaTime);

private:
	
	UPROPERTY()
	AOnlineShooterCharacter* Character;

	UPROPERTY()
	AOnlineShooterPlayerController* Controller;
	
	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;
	
	float DrawHitBoxTime = 0.f;

	UPROPERTY(EditAnywhere, Category = "Debug", meta = (EditCondition = "bDebug"))
	float DrawHitBoxFrequency = .5f;

	UPROPERTY(EditAnywhere, Category = "Debug", meta = (EditCondition = "bDebug"))
	float DrawHitBoxLifeTime = .5f;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebug = false;

	UPROPERTY()
	uint32 DeletedPackagesCount = 0;

protected:

	UFUNCTION()
	void SaveFramePackage(FFramePackage& Package);

	UFUNCTION()
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

public:

	UFUNCTION()
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color, float DeltaTime);

	UFUNCTION()
	void ServerSideRewind(AOnlineShooterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);

	
};
