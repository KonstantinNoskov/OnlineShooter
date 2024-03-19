﻿#pragma once

#include "CoreMinimal.h"
#include "OnlineShooterGameMode.h"
#include "TeamsGameMode.generated.h"


UCLASS()
class ONLINESHOOTER_API ATeamsGameMode : public AOnlineShooterGameMode
{
	GENERATED_BODY()

protected:
	virtual void HandleMatchHasStarted() override;
	virtual void OnPostLogin(AController* NewPlayer) override;

public:
	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
};
