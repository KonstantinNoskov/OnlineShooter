#include "OnlineShooterGameMode.h"

#include "Characters/OnlineShooterCharacter.h"

// Add libs
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

void AOnlineShooterGameMode::PlayerEliminated(AOnlineShooterCharacter* ElimedCharacter,
                                              AOnlineShooterPlayerController* VictimController, AOnlineShooterPlayerController* AttackerController)
{
	if (ElimedCharacter)
	{
		ElimedCharacter->Eliminated();
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
		// Get All respawn points on a map
		TArray<AActor*> RespawnPoints;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(),RespawnPoints);

		// Get All characters  on a map
		TArray<AActor*> AllCharacters;
		UGameplayStatics::GetAllActorsOfClass(this, AOnlineShooterCharacter::StaticClass(),AllCharacters);

		/*
		 * Now we want to make eliminated character spawn as far from all other characters as possible
		 *
		 * Most remote spawn point gonna be determined by summarizing magnitudes from all characters to it. 
		 */

		// Most remote spawn point
		AActor* MostDistancePoint = nullptr;
		
		// The highest magnitude
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
		
		int32 RandomRespawnPoint = FMath::RandRange(0, RespawnPoints.Num()-1);

		// if remote spawn point is valid 
		if (MostDistancePoint)
		{
			// Spawn eliminated player
			RestartPlayerAtPlayerStart(EliminatedController, MostDistancePoint);  

			UE_LOG(LogTemp, Warning, TEXT("MostDistancePoint:%s, MaxDistance:%f"), *MostDistancePoint->GetFName().ToString(), MaxDistance)
		}
	}
}
