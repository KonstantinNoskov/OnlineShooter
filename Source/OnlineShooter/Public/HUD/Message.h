#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Message.generated.h"


class UHorizontalBox;

UCLASS()
class ONLINESHOOTER_API UMessage : public UUserWidget
{
	GENERATED_BODY()

public:
	
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* MessageBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MessageText;
	
	
};
