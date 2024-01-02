// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "OnlineShooterGameMode.generated.h"

class AOnlineShooterPlayerController;
class AOnlineShooterCharacter;

namespace MatchState
{
	extern ONLINESHOOTER_API const FName Cooldown; // Match duration has been reached. Display winner and begin cooldown timer.
}

UCLASS()
class ONLINESHOOTER_API AOnlineShooterGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	AOnlineShooterGameMode();

	virtual void Tick(float DeltaSeconds) override;
	virtual void PlayerEliminated(AOnlineShooterCharacter* ElimedCharacter, AOnlineShooterPlayerController* VictimController, AOnlineShooterPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController);
	
	AActor* GetRespawnPoint();
	
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY()
	float LevelStartingTime = 0.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

protected:

	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	
	UPROPERTY()
	float CountdownTime = 10.f;

public:
	FORCEINLINE float GetCountdownTime() const { return CooldownTime; }
};
