#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SniperScopeWidget.generated.h"


class UImage;

UCLASS()
class ONLINESHOOTER_API USniperScopeWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	UImage* ScopeOverlay;

	UPROPERTY(meta = (BindWidget))
	UImage* Background;
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* ScopeZoomIn;
	
};
