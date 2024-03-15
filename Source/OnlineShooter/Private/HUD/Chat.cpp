#include "HUD/Chat.h"

#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "HUD/Message.h"

void UChat::ChatSetup()
{
	ShowChat();
	FocusChat();
	
}
void UChat::ShowChat() 
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);
}
void UChat::FocusChat() 
{
	ChatInput->SetFocus();
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

void UChat::AddChatMessageText(FString PublisherName, FString PlayerMessage)
{
	ChatMessage = CreateWidget<UMessage>(GetOwningPlayer(), ChatMessageClass);
	if (ChatScrollBox && ChatMessage)
	{
		FString ChatScrollText = FString::Printf(TEXT("%s: %s"), *PublisherName, *PlayerMessage);
		ChatMessage->MessageText->SetText(FText::FromString(ChatScrollText));
		ChatScrollBox->AddChild(ChatMessage);
	}
}