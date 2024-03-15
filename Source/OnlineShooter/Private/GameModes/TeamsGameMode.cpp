﻿#include "GameModes/TeamsGameMode.h"

#include "GameStates/OnlineShooterGameState.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerStates/OnlineShooterPlayerState.h"

void ATeamsGameMode::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	AOnlineShooterGameState* ShooterGameState = Cast<AOnlineShooterGameState>(UGameplayStatics::GetGameState(this));
	if (ShooterGameState)
	{

		AOnlineShooterPlayerState* OPlayerState = NewPlayer->GetPlayerState<AOnlineShooterPlayerState>();
		if (OPlayerState && OPlayerState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (ShooterGameState->BlueTeam.Num() >= ShooterGameState->RedTeam.Num())
			{
				ShooterGameState->RedTeam.AddUnique(OPlayerState);
				OPlayerState->SetTeam(ETeam::ET_RedTeam);
			}
		}
		else
		{
			ShooterGameState->BlueTeam.AddUnique(OPlayerState);
			OPlayerState->SetTeam(ETeam::ET_BlueTeam);
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	AOnlineShooterGameState* ShooterGameState = Cast<AOnlineShooterGameState>(UGameplayStatics::GetGameState(this));
	AOnlineShooterPlayerState* OPlayerState = Exiting->GetPlayerState<AOnlineShooterPlayerState>();
	if (ShooterGameState && OPlayerState)
	{
		if (ShooterGameState->RedTeam.Contains(OPlayerState))
		{
			ShooterGameState->RedTeam.Remove(OPlayerState);
		}

		if (ShooterGameState->BlueTeam.Contains(OPlayerState))
		{
			ShooterGameState->BlueTeam.Remove(OPlayerState);
		}
	}
}


void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	AOnlineShooterGameState* ShooterGameState = Cast<AOnlineShooterGameState>(UGameplayStatics::GetGameState(this));
	if (ShooterGameState)
	{
		for (auto PState : ShooterGameState->PlayerArray)
		{
			AOnlineShooterPlayerState* OPlayerState = Cast<AOnlineShooterPlayerState>(PState.Get());
			if (OPlayerState && OPlayerState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (ShooterGameState->BlueTeam.Num() >= ShooterGameState->RedTeam.Num())
				{
					ShooterGameState->RedTeam.AddUnique(OPlayerState);
					OPlayerState->SetTeam(ETeam::ET_RedTeam);
				}
			}
			else
			{
				ShooterGameState->BlueTeam.AddUnique(OPlayerState);
				OPlayerState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}


