#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MainMenuHUD.generated.h"


class UMainMenuWidget;

UCLASS()
class ONLINESHOOTER_API AMainMenuHUD : public AHUD
{
	GENERATED_BODY()

	AMainMenuHUD();
	
public:
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMainMenuWidget> MainMenuClass;

	UPROPERTY()
	UMainMenuWidget* MainMenuOverlay;

	UPROPERTY()
	UMainMenuWidget* CacheOverlay;

	UFUNCTION()
	void AddMainMenu();


public:

	UFUNCTION(BlueprintPure)
	FORCEINLINE UMainMenuWidget* GetMainMenuOverlay() const { return MainMenuOverlay; }
	
};
