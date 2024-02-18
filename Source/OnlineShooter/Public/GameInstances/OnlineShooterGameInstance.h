#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineShooterGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FMapInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FString MapURL;

	UPROPERTY(EditAnywhere)
	FText MapName;

	UPROPERTY(EditAnywhere)
	FText MapDescription;

	UPROPERTY(EditAnywhere)
	UTexture2D* MapImage;

	// Operators Overload
	FORCEINLINE bool operator == (const FMapInfo& Other) const
	{
		return  MapURL == Other.MapURL;
	}
	FORCEINLINE void operator = (const FMapInfo& Other)
	{
		MapURL = Other.MapURL;
		MapDescription = Other.MapDescription;
		MapName = Other.MapName;
		MapImage = Other.MapImage;
	}
};

UCLASS()
class ONLINESHOOTER_API UOnlineShooterGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UFUNCTION()
	void GoToMap(const FString& InMapURL); 
};
