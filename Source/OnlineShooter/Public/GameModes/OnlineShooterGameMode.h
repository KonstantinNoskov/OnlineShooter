// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "OnlineShooterGameMode.generated.h"

class AOnlineShooterPlayerController;
class AOnlineShooterCharacter;


UCLASS()
class ONLINESHOOTER_API AOnlineShooterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	
	virtual void PlayerEliminated(AOnlineShooterCharacter* ElimedCharacter, AOnlineShooterPlayerController* VictimController, AOnlineShooterPlayerController* AttackerController);

	AActor* GetRespawnPoint();
	virtual void RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController);
};
