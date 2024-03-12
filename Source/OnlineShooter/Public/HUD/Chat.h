#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Chat.generated.h"


class UEditableTextBox;
class UScrollBox;

UCLASS()
class ONLINESHOOTER_API UChat : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta = (BindWidget))
	UScrollBox* ChatScrollBox;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* ChatInput;

	UPROPERTY()
	APlayerController* PlayerController;

public:
	
	void ChatSetup();
	
	void ChatTearDown();
	
	FORCEINLINE UEditableTextBox* GetChatInput() const { return ChatInput; }

	
};
