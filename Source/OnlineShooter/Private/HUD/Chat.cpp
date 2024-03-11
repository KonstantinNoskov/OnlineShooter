#include "HUD/Chat.h"

#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

void UChat::SetChatMessageText(FString ChatMessage)
{
	FString Message = FString::Printf(TEXT("%s"), *ChatMessage);
	if (MessageText)
	{
		MessageText->SetText(FText::FromString(Message));
	}
}

void UChat::ChatSetup()
{
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
}

void UChat::ChatTearDown()
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
}

bool UChat::Initialize()
{
	if (!Super::Initialize()) return false;

	
	return true;
}
