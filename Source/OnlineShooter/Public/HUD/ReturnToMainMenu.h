#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenu.generated.h"


class UMultiplayerSessionsSubsystem;
class UButton;

UCLASS()
class ONLINESHOOTER_API UReturnToMainMenu : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual bool Initialize() override;

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);


private:
	UPROPERTY(meta = (BindWidget))
	UButton* ReturnButton;

	UPROPERTY()
	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	UPROPERTY()
	APlayerController* PlayerController;
	
	UFUNCTION()
	void ReturnButtonClicked();

#pragma region LEAVING SESSION

protected:

	UFUNCTION()
	void OnPlayerLeftGame();
	
#pragma endregion
	
public:

	void MenuSetup();
	void MenuTearDown();
	
};
