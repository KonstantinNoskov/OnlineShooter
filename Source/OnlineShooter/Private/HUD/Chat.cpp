#include "HUD/Chat.h"

#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "HUD/Message.h"



void UChat::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	ChatHideOnTime(InDeltaTime);
}

void UChat::ChatSetup()
{
	ShowChat();
	FocusChat();
}
void UChat::ShowChat() 
{
	SetVisibility(ESlateVisibility::Visible);
	bChatOpen = true;
}

void UChat::HideChat()
{	
	SetVisibility(ESlateVisibility::Hidden);
	bChatOpen = false;
	ChatHideCurrentTime = 0.f;
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

void UChat::ChatHideOnTime(float DeltaTime)
{
	ChatHideCurrentTime += DeltaTime;

	UE_LOG(LogTemp, Warning, TEXT("%f"), ChatHideCurrentTime);

	if (ChatHideCurrentTime > ChatHideThreshold && IsChatOpen())
	{
		PlayerController = !PlayerController ? GetOwningPlayer() : PlayerController;

		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			bChatOpen = false;
			ChatHideCurrentTime = 0.f;
			SetVisibility(ESlateVisibility::Hidden);
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


