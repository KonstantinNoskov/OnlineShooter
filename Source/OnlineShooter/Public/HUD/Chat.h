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


protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	friend class AOnlineShooterHUD;

private:
	UPROPERTY()
	APlayerController* PlayerController;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UMessage> ChatMessageClass;

	UPROPERTY()
	bool bChatOpen = false;

	UPROPERTY(EditAnywhere)
	float ChatHideThreshold = 5.f;
	float ChatHideCurrentTime = 0.f;

	UFUNCTION()
	void ChatHideOnTime(float DeltaTime);

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
	void HideChat();
	void ChatSetup();
	
	void ChatTearDown();

	FORCEINLINE void ClearInput() { ChatInput->SetText(FText()); }
	FORCEINLINE UEditableTextBox* GetChatInput() const { return ChatInput; }
	FORCEINLINE bool IsChatOpen() const { return bChatOpen; }
	FORCEINLINE void SetChatOpen(bool NewChatOpen) { bChatOpen = NewChatOpen; }
	FORCEINLINE void ResetChatTimer() { ChatHideCurrentTime = 0.f; }


};
