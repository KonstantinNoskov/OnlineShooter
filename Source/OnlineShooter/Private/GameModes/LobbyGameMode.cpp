#include "GameModes/LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// Store the amount of players joined the game
	 int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	// Travel listen server to chosen level map when lobby is full 
	if(NumberOfPlayers == PlayersAmountToStart)
	{
		FString PathToGameReady = FString::Printf(TEXT("%s?listen"), *MapToTravel);
		UWorld* World = GetWorld();

		if(World)
		{
			bUseSeamlessTravel = true;
			World->ServerTravel(PathToGameReady);
		}
	}
}
