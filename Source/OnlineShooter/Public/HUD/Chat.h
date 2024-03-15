#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h"
#include "Chat.generated.h"

class UVerticalBox;
class UCanvasPanelSlot;
class UMessage;
class UTextBlock;
class UEditableTextBox;
class UScrollBox;

UCLASS()
class ONLINESHOOTER_API UChat : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY()
	APlayerController* PlayerController;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UMessage> ChatMessageClass;

	UPROPERTY()
	bool bChatOpen = false;

public:

	
	
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ChatScrollBox;
	
	UPROPERTY()
	UMessage* ChatMessage;
	
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* ChatInput; 

	UFUNCTION()
	void AddChatMessageText(FString PublisherName, FString PlayerMessage);

	void FocusChat();
	void ShowChat();
	void ChatSetup();
	
	void ChatTearDown();

	FORCEINLINE void ClearInput() { ChatInput->SetText(FText()); }
	FORCEINLINE UEditableTextBox* GetChatInput() const { return ChatInput; }
	FORCEINLINE bool IsChatOpen() const { return bChatOpen; }
	FORCEINLINE void SetChatOpen(bool NewChatOpen) { bChatOpen = NewChatOpen; }
};
