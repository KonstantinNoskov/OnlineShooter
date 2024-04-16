#include "HUD/ReturnToMainMenu.h"

#include "MultiplayerSessionsSubsystem.h"
#include "Characters/OnlineShooterCharacter.h"
#include "Components/Button.h"
#include "GameFramework/GameModeBase.h"


bool UReturnToMainMenu::Initialize()
{
	if (!Super::Initialize()) return false;

	// Bound ReturnButtonClicked delegate
	if (ReturnButton && !ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
	}
	
	return true;
}

void UReturnToMainMenu::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = !PlayerController ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	if (ReturnButton) 
	{
		ReturnButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
	}
	
	// Bound OnDestroySession delegate
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem/* && !MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionsComplete.IsBound()*/) 
		{
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionsComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroySession);
		}
	}
}

void UReturnToMainMenu::OnDestroySession(bool bWasSuccessful) 
{
	if (!bWasSuccessful)
	{
		ReturnButton->SetIsEnabled(true);
		
		return;
	}
	
	UWorld* World = GetWorld();
	if (World)
	{	
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		if (GameMode)
		{
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			PlayerController = !PlayerController ? World->GetFirstPlayerController() : PlayerController;
			if (PlayerController)
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
}

void UReturnToMainMenu::MenuTearDown() 
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
						
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}

	// Unbound ReturnButtonClicked delegate
	if (ReturnButton && ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.RemoveDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
	}

	// Unbound OnDestroySession delegate
	if (MultiplayerSessionsSubsystem /*&& MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionsComplete.IsBound()*/)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionsComplete.RemoveDynamic(this, &UReturnToMainMenu::OnDestroySession);
	}
}

void UReturnToMainMenu::ReturnButtonClicked()
{
	ReturnButton->SetIsEnabled(false);
	
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* FirstPlayerController = World->GetFirstPlayerController();
		if (FirstPlayerController)
		{
			AOnlineShooterCharacter* OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(FirstPlayerController->GetPawn());
			if (OnlineShooterCharacter)
			{
				OnlineShooterCharacter->ServerLeaveGame();
				OnlineShooterCharacter->OnLeftGame.AddDynamic(this, &UReturnToMainMenu::OnPlayerLeftGame);
				RemoveFromParent();
			}
			else
			{
				ReturnButton->SetIsEnabled(true);
			}
		}
	}
}

#pragma region LEAVING SESSION

void UReturnToMainMenu::OnPlayerLeftGame()
{
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->DestroySession();
		
	}
}

#pragma endregion
