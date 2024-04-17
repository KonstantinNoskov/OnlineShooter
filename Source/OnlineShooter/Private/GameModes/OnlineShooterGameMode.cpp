#include "GameModes/OnlineShooterGameMode.h"

#include "Characters/OnlineShooterCharacter.h"

// Add libs
#include "NiagaraComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameStates/OnlineShooterGameState.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/OnlineShooterPlayerController.h"
#include "PlayerStates/OnlineShooterPlayerState.h"

namespace MatchState 
{
	const FName Cooldown = FName("Cooldown");
}

AOnlineShooterGameMode::AOnlineShooterGameMode()
{
	bDelayedStart = true;
}

void AOnlineShooterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AOnlineShooterGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;

		if(CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}

	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if(CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown); 
		}
	}

	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;

		if(CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void AOnlineShooterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AOnlineShooterPlayerController* OnlineShooterPlayer = Cast<AOnlineShooterPlayerController>(*It);

		if(OnlineShooterPlayer)
		{
			OnlineShooterPlayer->OnMatchStateSet(MatchState, bTeamsMatch);
		}
	}
}

void AOnlineShooterGameMode::PlayerEliminated(AOnlineShooterCharacter* ElimedCharacter,
                                              AOnlineShooterPlayerController* VictimController, AOnlineShooterPlayerController* AttackerController)
{
	AOnlineShooterPlayerState* AttackerPlayerState = AttackerController ? Cast<AOnlineShooterPlayerState>(AttackerController->PlayerState) : nullptr;
	AOnlineShooterPlayerState* VictimPlayerState = VictimController ? Cast<AOnlineShooterPlayerState>(VictimController->PlayerState) : nullptr;

	AOnlineShooterGameState* OnlineShooterGameState = GetGameState<AOnlineShooterGameState>();
	
	// If player get a kill - add 1 frag to his score & update top score
	// Activate leading crown effect if Attacker gained the lead and deactivate effect for those who lost one
	if(AttackerPlayerState && AttackerPlayerState != VictimPlayerState && OnlineShooterGameState)
	{

		// Store currently leading players
		TArray<AOnlineShooterPlayerState*> PlayersCurrentlyInTheLead;
		for (auto LeadPlayer : OnlineShooterGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}	
		
		AttackerPlayerState->AddToScore(1.f);
		OnlineShooterGameState->UpdateTopScore(AttackerPlayerState);
		
		// Looking for those player's who gained the lead
		if (OnlineShooterGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			AOnlineShooterCharacter* Leader = Cast<AOnlineShooterCharacter>(AttackerPlayerState->GetPawn());
			if (Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}

		// Looking for those player's whom lost the lead
		for (int32 i = 0; i < PlayersCurrentlyInTheLead.Num(); i++)
		{
			if (!OnlineShooterGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))
			{
			 	AOnlineShooterCharacter* Loser = Cast<AOnlineShooterCharacter>(PlayersCurrentlyInTheLead[i]->GetPawn());
			    if (Loser)
			    {
				    Loser->MulticastLostTheLead();
			    }
			}
		}
	}

	// At the same time increment VictimPlayer defeat count
	if(VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	// Activate eliminate logic on VictimPlayer
	if (ElimedCharacter)
	{
		ElimedCharacter->Eliminated(false);
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AOnlineShooterPlayerController* OnlineShooterPlayer = Cast<AOnlineShooterPlayerController>(*It);
		if (OnlineShooterPlayer && AttackerPlayerState && VictimPlayerState)
		{
			OnlineShooterPlayer->BroadcastEliminate(AttackerPlayerState, VictimPlayerState);
		}
	}
}

void AOnlineShooterGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
	if(EliminatedCharacter)
	{
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}
	
	if(EliminatedController)
	{
		// if remote spawn point is valid 
		AActor* MostRemoteStartPoint = GetRespawnPoint();
		if (MostRemoteStartPoint)
		{
			// Spawn eliminated player
			RestartPlayerAtPlayerStart(EliminatedController, MostRemoteStartPoint);
			AOnlineShooterCharacter* OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(EliminatedController->GetPawn());
			if (OnlineShooterCharacter)
			{
				OnlineShooterCharacter->UpdateHUDGrenades();
				OnlineShooterCharacter->UpdateHUDAmmo();
				OnlineShooterCharacter->UpdateHUDShield();
			}
		}
	}
}

#pragma region LEAVING SESSION

void AOnlineShooterGameMode::PlayerLeftGame(AOnlineShooterPlayerState* PlayerLeaving)
{
	if (!PlayerLeaving) return;
		
	AOnlineShooterGameState* OnlineShooterGameState = GetGameState<AOnlineShooterGameState>();
	if (OnlineShooterGameState && OnlineShooterGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		OnlineShooterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	
	AOnlineShooterCharacter* CharacterLeaving = Cast<AOnlineShooterCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Eliminated(true);
	}
}

#pragma endregion

#pragma region TEAMS

float AOnlineShooterGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) 
{
	return BaseDamage;
}

#pragma endregion

AActor* AOnlineShooterGameMode::GetRespawnPoint()
{	
	/*
	 * We want to make eliminated character spawn as far as possible from the all other characters
	 * Most remote spawn point have to be determined by summarizing magnitudes from all characters to it. 
	 */
	
	AActor* MostDistancePoint = nullptr;
	float MaxDistance = 0.f;
	
	// Get All respawn points on the map
	TArray<AActor*> RespawnPoints;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(),RespawnPoints);

	// Return nullptr if there's no respawn points on the map
	if (RespawnPoints.IsEmpty()) return MostDistancePoint;
	
	// Get All characters on the map
	TArray<AActor*> AllCharacters;
	UGameplayStatics::GetAllActorsOfClass(this, AOnlineShooterCharacter::StaticClass(),AllCharacters);
	
	// Return any random spawn point in case the character is only one on the map
	if (AllCharacters.IsEmpty()) return MostDistancePoint = RespawnPoints[FMath::RandRange(0, RespawnPoints.Num()-1)];
	
	// Run through all potential spawn points
	for (AActor* RespawnPoint : RespawnPoints)
	{
		// Potential spawn point location
		FVector RespawnPointLocation = RespawnPoint->GetActorLocation();

		// Sum of all magnitudes from potential spawn point to all players on the map 
		float SumDistance = 0.f;

		// Run through all characters on the map
		for (AActor* Character : AllCharacters)
		{
			// Get current character location
			FVector CharacterLocation = Character->GetActorLocation();

			// summarize magnitudes from all characters to potential point
			SumDistance += (RespawnPointLocation - CharacterLocation).Size();
		}

		// if current magnitude sum greater than max magnitude
		if(MaxDistance < SumDistance)
		{
			// Override max magnitude
			MaxDistance = SumDistance;

			// Override most remote spawn point
			MostDistancePoint = RespawnPoint; 
		}
		
	}
	
	return MostDistancePoint;
}