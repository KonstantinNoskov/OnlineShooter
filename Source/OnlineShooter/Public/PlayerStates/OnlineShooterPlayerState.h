#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "OnlineShooterPlayerState.generated.h"


class AOnlineShooterPlayerController;
class AOnlineShooterCharacter;

UCLASS()
class ONLINESHOOTER_API AOnlineShooterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	
	virtual void OnRep_Score() override;
	virtual  void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	
	UFUNCTION()
	virtual void OnRep_Defeats();

	UFUNCTION()
	void AddToScore(float ScoreAmount);

	UFUNCTION()
	void AddToDefeats(int32 DefeatsAmount);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetAttackerName(const FString& Name);

	UFUNCTION()
	void SetAttackerName(FString Name);

	
private:

	UPROPERTY()
	AOnlineShooterCharacter* Character;

	UPROPERTY()
	AOnlineShooterPlayerController* Controller;

	UPROPERTY(ReplicatedUsing=OnRep_Defeats)
	int32 Defeats = 0;

	UPROPERTY()
	FString KilledBy;
};
