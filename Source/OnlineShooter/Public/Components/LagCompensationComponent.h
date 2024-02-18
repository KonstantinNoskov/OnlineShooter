#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

class AOnlineShooterCharacter;
class AWeapon;
class AOnlineShooterPlayerController;

USTRUCT(BlueprintType)
struct FBoxInformation 
{
	GENERATED_BODY()

	UPROPERTY(meta = (IgnoreForMemberInitializationTest))
	FVector Location;

	UPROPERTY(meta = (IgnoreForMemberInitializationTest))
	FRotator Rotation;

	UPROPERTY(meta = (IgnoreForMemberInitializationTest))
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY(meta = (IgnoreForMemberInitializationTest))
	float Time;

	UPROPERTY(meta = (IgnoreForMemberInitializationTest))
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY(meta = (IgnoreForMemberInitializationTest))
	AOnlineShooterCharacter* Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY(meta = (IgnoreForMemberInitializationTest))
	bool bHitConfirmed;

	UPROPERTY(meta = (IgnoreForMemberInitializationTest))
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY(meta = (IgnoreForMemberInitializationTest))
	TMap<AOnlineShooterCharacter*, uint32> HeadShots;

	UPROPERTY(meta = (IgnoreForMemberInitializationTest))
	TMap<AOnlineShooterCharacter*, uint32> BodyShots;
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

	// Shotgun

	FFramePackage GetFrameToCheck(AOnlineShooterCharacter* HitCharacter, float HitTime);

	UFUNCTION()
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	
	UFUNCTION()
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color, float DeltaTime);

	UFUNCTION()
	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package,
		AOnlineShooterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation);

	UFUNCTION()
	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations);

	UFUNCTION()
	void CacheBoxPositions(AOnlineShooterCharacter* HitCharacter, FFramePackage& OutFramePackage);

	UFUNCTION()
	void MoveBoxes(AOnlineShooterCharacter* HitCharacter, const FFramePackage& InPackage);

	UFUNCTION()
	void ResetHitBoxes(AOnlineShooterCharacter* HitCharacter, const FFramePackage& InPackage);

	UFUNCTION()
	void EnableCharacterMeshCollision(AOnlineShooterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	
public:
	
	UFUNCTION()
	FServerSideRewindResult ServerSideRewind(
		AOnlineShooterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime);

	UFUNCTION()
	FShotgunServerSideRewindResult ShotgunServerSideRewind(
		const TArray<AOnlineShooterCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);

	
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(
		AOnlineShooterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime, AWeapon* DamageCauser);

	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(
		const TArray<AOnlineShooterCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime, AWeapon* DamageCauser);

	
};






