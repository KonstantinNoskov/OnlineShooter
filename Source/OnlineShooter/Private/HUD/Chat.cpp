#include "HUD/Chat.h"

#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

void UChat::FocusChat()
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

			ChatInput->SetFocus();
		}
	}
}

void UChat::ShowChat()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);
}

void UChat::ChatSetup()
{
	ShowChat();
	
	FocusChat();
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


void UChat::SetChatScrollText(FString PublisherName, FString PlayerMessage)
{
	FString ChatScrollText = FString::Printf(TEXT("%s: %s"), *PublisherName, *PlayerMessage);
	if (ChatScrollTextBlock)
	{
		ChatScrollTextBlock->SetText(FText::FromString(ChatScrollText));
	}
}