#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "OnlineShooterGameState.generated.h"


class AOnlineShooterPlayerState;

UCLASS()
class ONLINESHOOTER_API AOnlineShooterGameState : public AGameState
{
	GENERATED_BODY()

public:
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(AOnlineShooterPlayerState* ScoringPlayer);
	
	UPROPERTY(Replicated)
	TArray<AOnlineShooterPlayerState*> TopScoringPlayers;

private:

	float TopScore = 0.f;
};
