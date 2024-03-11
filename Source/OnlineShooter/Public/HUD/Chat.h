#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Chat.generated.h"


class UEditableTextBox;
class UTextBlock;
class UHorizontalBox;

UCLASS()
class ONLINESHOOTER_API UChat : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

private:
	
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* MessageBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MessageText;

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* InputBox;
	
	UPROPERTY()
	APlayerController* PlayerController;
	
	void SetChatMessageText(FString ChatMessage);


public:
	
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* InputText;
	
	UFUNCTION()
	void ChatSetup();

	UFUNCTION(BlueprintCallable)
	void ChatTearDown();

	

};
 