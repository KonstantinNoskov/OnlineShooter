// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameInstances/OnlineShooterGameInstance.h"
#include "MainMenuWidget.generated.h"


class USoloMapButton;
class AMainMenuHUD;
class UCanvasPanel;
class UVerticalBox;
class UTextBlock;
class UButton;
class UWidgetSwitcher;
class UImage;

UCLASS()
class ONLINESHOOTER_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

#pragma region OVERRIDES
	

protected:
	virtual void NativeOnInitialized() override;
	
	virtual void NativeConstruct() override;
	


#pragma endregion

private:
	UPROPERTY()
	AMainMenuHUD* MainMenuHUD;

	UPROPERTY()
	APlayerController* Controller;

	
#pragma region MAIN

public:
	
	// Set Default values
	void SetMapsInfo();

private:
	
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* BackgroundSwitch;
	
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* MainOverlay;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* SoloOverlay;
	
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* MainVerticalBox;
	
	// Solo Button
	UPROPERTY(meta = (BindWidget))
	UButton* SoloButton;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SoloButtonText;

	// Multiplayer Button
	UPROPERTY(meta = (BindWidget))
	UButton* MultiplayerButton;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MultiplayerButtonText;

	// Options Button
	UPROPERTY(meta = (BindWidget))
	UButton* OptionsButton;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* OptionsButtonText;

	// Quit Button
	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuitButtonText;

	// Colors
	UPROPERTY(EditAnywhere, Category = "Text Color")
	FLinearColor ButtonTextInitialColor;

	UPROPERTY(EditAnywhere, Category = "Text Color")
	FLinearColor ButtonTextHoverColor;
	
	// Clicked
	UFUNCTION()
	void SoloButtonClicked();
	UFUNCTION()
	void MultiplayerButtonClicked();
	UFUNCTION()
	void OptionsButtonClicked();
	UFUNCTION()
	void QuitButtonClicked();

	// Hovered
	UFUNCTION()
	void SoloButtonHovered();
	UFUNCTION()
	void MultiplayerButtonHovered();
	UFUNCTION()
	void OptionsButtonHovered();
	UFUNCTION()
	void QuitButtonHovered();

	// Unhovered
	UFUNCTION()
	void SoloButtonUnhovered();
	UFUNCTION()
	void MultiplayerButtonUnhovered();
	UFUNCTION()
	void OptionsButtonUnhovered();
	UFUNCTION()
	void QuitButtonUnhovered();
	
	// Switch Overlays
	void SwitchToSolo();

public:
	
	// Animations
	UPROPERTY(BlueprintReadWrite, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* ButtonSlideInAnim;

	UPROPERTY(BlueprintReadWrite, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* ButtonSlideOutAnim;

	FWidgetAnimationDynamicEvent SlideInAnimFinished;

	UFUNCTION()
	void OnSlideInAnimFinished();
	
private:

	UPROPERTY(EditAnywhere)
	float ButtonSlideInSpeed = 2.f; 

#pragma endregion

#pragma region SOLO

private:
	UPROPERTY(EditAnywhere, Category = "Maps")
	TArray<FMapInfo> MapList;

	UPROPERTY(EditAnywhere, Category = "Maps")
	TSubclassOf<USoloMapButton> SoloMapButtonClass;
	
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* SoloMapList;

	UPROPERTY(meta = (BindWidget))
	UImage* SoloMapImage;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SoloMapDescription;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentMapName;
	
	UPROPERTY(meta = (BindWidget))
	UButton* SoloBackButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SoloBackButtonText;

	// Clicked
	UFUNCTION()
	void SoloBackButtonClicked();

public:

	UFUNCTION()
	void UpdateCurrentMapInfo(const FMapInfo& InMapInfo);

#pragma endregion

#pragma region MENU SETUP
	
public:

	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void MainMenuSetup();

public:
#pragma endregion

	
};
