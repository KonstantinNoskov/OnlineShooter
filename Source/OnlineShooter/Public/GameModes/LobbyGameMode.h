#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ONLINESHOOTER_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

private:

	UPROPERTY(EditAnywhere, Category = "Network")
	uint32 PlayersAmountToStart = 2;

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

	UPROPERTY(EditDefaultsOnly, Category = "Network")
	FString MapToTravel{TEXT("/Game/Maps/Highrise")};
};
