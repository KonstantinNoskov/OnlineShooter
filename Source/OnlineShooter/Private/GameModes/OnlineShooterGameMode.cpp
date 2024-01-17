#include "GameModes/OnlineShooterGameMode.h"

#include "Characters/OnlineShooterCharacter.h"

// Add libs
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
			OnlineShooterPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void AOnlineShooterGameMode::PlayerEliminated(AOnlineShooterCharacter* ElimedCharacter,
                                              AOnlineShooterPlayerController* VictimController, AOnlineShooterPlayerController* AttackerController)
{
	AOnlineShooterPlayerState* AttackerPlayerState = AttackerController ? Cast<AOnlineShooterPlayerState>(AttackerController->PlayerState) : nullptr;
	AOnlineShooterPlayerState* VictimPlayerState = VictimController ? Cast<AOnlineShooterPlayerState>(VictimController->PlayerState) : nullptr;

	AOnlineShooterGameState* OnlineShooterGameState = GetGameState<AOnlineShooterGameState>();
	
	if(AttackerPlayerState && AttackerPlayerState != VictimPlayerState && OnlineShooterGameState)
	{
		AttackerPlayerState->AddToScore(1.f);

		OnlineShooterGameState->UpdateTopScore(AttackerPlayerState);
	}

	if(VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	
	if (ElimedCharacter)
	{
		ElimedCharacter->Eliminated();
	}
}

AActor* AOnlineShooterGameMode::GetRespawnPoint()
{	
	/*
	 * We want to make eliminated character spawn as far as possible from the all other characters
	 *
	 * Most remote spawn point gonna be determined by summarizing magnitudes from all characters to it. 
	 */

	// Get All respawn points on a map
	TArray<AActor*> RespawnPoints;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(),RespawnPoints);

	// Get All characters  on a map
	TArray<AActor*> AllCharacters;
	UGameplayStatics::GetAllActorsOfClass(this, AOnlineShooterCharacter::StaticClass(),AllCharacters);

	AActor* MostDistancePoint = nullptr;
	float MaxDistance = 0.f;

	// Run through all potential spawn points
	for (AActor* RespawnPoint : RespawnPoints)
	{
		// potential spawn point location
		FVector RespawnPointLocation = RespawnPoint->GetActorLocation();

		// Magnitudes Sum
		float SumDistance = 0.f;

		// run through all characters on a map
		for (AActor* Character : AllCharacters)
		{
			// get current character location
			FVector CharacterLocation = Character->GetActorLocation();

			// summarize magnitudes from all character to potential point
			SumDistance += (RespawnPointLocation - CharacterLocation).Size();
		}

		// if current magnitude sum higher than max magnitude
		if(MaxDistance < SumDistance)
		{
			MaxDistance = SumDistance; // Override max magnitude
			MostDistancePoint = RespawnPoint; // Override most remote spawn point
		}
	}

	//UE_LOG(LogTemp, Warning, TEXT("MostDistancePoint:%s, MaxDistance:%f"), *MostDistancePoint->GetFName().ToString(), MaxDistance)
	
	return MostDistancePoint;
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
		AActor* MostDistancePoint = GetRespawnPoint();
		
		// if remote spawn point is valid 
		if (MostDistancePoint)
		{
			// Spawn eliminated player
			RestartPlayerAtPlayerStart(EliminatedController, MostDistancePoint);  
		}
	}
}
