#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Chat.generated.h"


class UTextBlock;
class UEditableTextBox;
class UScrollBox;

UCLASS()
class ONLINESHOOTER_API UChat : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY()
	APlayerController* PlayerController;

public:

	UPROPERTY(meta = (BindWidget))
	UScrollBox* ChatScrollBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ChatScrollTextBlock;
	
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* ChatInput; 

	UFUNCTION()
	void SetChatScrollText(FString PublisherName, FString PlayerMessage);

	void FocusChat();
	void ShowChat();
	void ChatSetup();
	void ChatTearDown();
	
	FORCEINLINE UEditableTextBox* GetChatInput() const { return ChatInput; } 

	
};
