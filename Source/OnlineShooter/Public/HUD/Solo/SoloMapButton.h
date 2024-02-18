#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameInstances/OnlineShooterGameInstance.h"
#include "SoloMapButton.generated.h"

class UMainMenuWidget;
class UTextBlock;
class UButton;

UCLASS()
class ONLINESHOOTER_API USoloMapButton : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	void TravelToMap(const FString& InMapURL) const;

private:
	
	UPROPERTY(meta = (BindWidget))
	UButton* MapButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MapButtonText;

	UPROPERTY()
	UMainMenuWidget* ParentWidget;

public:
	
	UPROPERTY(EditAnywhere)
	FMapInfo Info;

	UFUNCTION()
	void SoloMapButtonOnClicked();

	UFUNCTION()
	void SoloMapButtonOnHovered();
	
	FORCEINLINE void SetParentWidget(UMainMenuWidget* NewMainMenu) { ParentWidget = NewMainMenu; }
	
};
