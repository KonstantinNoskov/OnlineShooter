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

#pragma region TEAMS
	
	TArray<AOnlineShooterPlayerState*> RedTeam;
	TArray<AOnlineShooterPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

	UFUNCTION()
	void OnRep_RedTeamScore();

	UFUNCTION()
	void OnRep_BlueTeamScore();
	
#pragma endregion

private:

	float TopScore = 0.f;
};
