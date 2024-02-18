#include "HUD/MainMenuWidget.h"

#include "Animation/WidgetAnimation.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Components/VerticalBox.h"
#include "HUD/MainMenuHUD.h"
#include "HUD/Solo/SoloMapButton.h"



void UMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Click 
	if(SoloButton)			{ SoloButton->OnClicked.AddDynamic(this,			&ThisClass::SoloButtonClicked);			}
	if(MultiplayerButton)	{ MultiplayerButton->OnClicked.AddDynamic(this, 	&ThisClass::MultiplayerButtonClicked);	}
	if(OptionsButton)		{ OptionsButton->OnClicked.AddDynamic(this,			&ThisClass::OptionsButtonClicked);		}
	if(QuitButton)			{ QuitButton->OnClicked.AddDynamic(this,			&ThisClass::QuitButtonClicked);			}
	if(SoloBackButton)		{ SoloBackButton->OnClicked.AddDynamic(this,		&ThisClass::SoloBackButtonClicked);		}

	// Hover
	if(SoloButton)			{ SoloButton->OnHovered.AddDynamic(this,			&ThisClass::SoloButtonHovered);			}
	if(MultiplayerButton)	{ MultiplayerButton->OnHovered.AddDynamic(this,		&ThisClass::MultiplayerButtonHovered);	}
	if(OptionsButton)		{ OptionsButton->OnHovered.AddDynamic(this,			&ThisClass::OptionsButtonHovered);		}
	if(QuitButton)			{ QuitButton->OnHovered.AddDynamic(this,			&ThisClass::QuitButtonHovered);			}
 
	// Unhovered 
	if(SoloButton)			{ SoloButton->OnUnhovered.AddDynamic(this,			&ThisClass::SoloButtonUnhovered);		}
	if(MultiplayerButton)	{ MultiplayerButton->OnUnhovered.AddDynamic(this,	&ThisClass::MultiplayerButtonUnhovered);}
	if(OptionsButton)		{ OptionsButton->OnUnhovered.AddDynamic(this,		&ThisClass::OptionsButtonUnhovered);	}
	if(QuitButton)			{ QuitButton->OnUnhovered.AddDynamic(this,			&ThisClass::QuitButtonUnhovered);		}
	
	// Animation 
	SlideInAnimFinished.BindDynamic(this, &ThisClass::OnSlideInAnimFinished);
	BindToAnimationFinished(ButtonSlideInAnim, SlideInAnimFinished);
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	/*if (ButtonSlideInAnim)
	{
		PlayAnimation(ButtonSlideInAnim, 0.f, 1.f, EUMGSequencePlayMode::Forward, ButtonSlideInSpeed);
	}*/
}

void UMainMenuWidget::OnSlideInAnimFinished()
{
	if (!BackgroundSwitch || !SoloOverlay) return;

	SwitchToSolo();
}


#pragma region MAIN

// Clicked
void UMainMenuWidget::SoloButtonClicked()
{

	if (ButtonSlideInAnim)
	{
		PlayAnimationReverse(ButtonSlideInAnim, 2.f, true);
	}

	
	/*if (!BackgroundSwitch || !SoloOverlay) return;

	// Switch to solo overlay
	BackgroundSwitch->SetActiveWidget(SoloOverlay);

	// Update Solo Maps values
	SetMapsInfo();

	// Update Solo Maps HUD 
	if (!MapList.IsEmpty())
	{
		UpdateCurrentMapInfo(MapList[0]);	
	}*/
}
void UMainMenuWidget::MultiplayerButtonClicked()
{
	
}
void UMainMenuWidget::OptionsButtonClicked()
{
}
void UMainMenuWidget::QuitButtonClicked()
{
}

// Hovered
void UMainMenuWidget::SoloButtonHovered()
{
	SoloButtonText->SetColorAndOpacity(ButtonTextHoverColor);
}
void UMainMenuWidget::MultiplayerButtonHovered()
{
	
}
void UMainMenuWidget::OptionsButtonHovered()
{
}
void UMainMenuWidget::QuitButtonHovered()
{
}

// Unhovered
void UMainMenuWidget::SoloButtonUnhovered()
{	
	SoloButtonText->SetColorAndOpacity(ButtonTextInitialColor);
}
void UMainMenuWidget::MultiplayerButtonUnhovered()
{
	
}
void UMainMenuWidget::OptionsButtonUnhovered()
{
}
void UMainMenuWidget::QuitButtonUnhovered()
{
}

// Switch Overlays
void UMainMenuWidget::SwitchToSolo()
{
	// Switch to solo overlay
	BackgroundSwitch->SetActiveWidget(SoloOverlay);

	// Update Solo Maps values
	SetMapsInfo();

	// Update Solo Maps HUD 
	if (!MapList.IsEmpty())
	{
		UpdateCurrentMapInfo(MapList[0]);	
	}
}

#pragma endregion
#pragma region SOLO

void UMainMenuWidget::SoloBackButtonClicked()
{
	// Valid Check
	if(BackgroundSwitch && MainOverlay)
	{
		// Switch back to Main Menu Overlay
		BackgroundSwitch->SetActiveWidget(MainOverlay);

		// Clear all maps from the map list to avoid the re-adding of the same maps
		SoloMapList->ClearChildren();
	}
}

void UMainMenuWidget::SetMapsInfo() 
{
	// Valid check
	Controller = !Controller ? Cast<APlayerController>(GetOwningPlayer()) : Controller;
	if (Controller && !MapList.IsEmpty())
	{
		// Run through all maps
		for (FMapInfo& MapInfo : MapList)
		{
			// Create buttons for each map in the map list
			USoloMapButton* SoloMapButton = CreateWidget<USoloMapButton>(Controller, SoloMapButtonClass);
			if(SoloMapButton)
			{	
				SoloMapButton->Info = MapInfo; // pass map info to Map Button 
				SoloMapButton->SetParentWidget(this); // Set MainMenu as a Parent Widget
			}

			// Add Map Buttons to a Map List Vertical Box
			if(SoloMapList)
			{	
				SoloMapList->AddChild(SoloMapButton);
			}	
		}
	}
}

void UMainMenuWidget::UpdateCurrentMapInfo(const FMapInfo& InMapInfo)
{
	SoloMapImage->SetBrushFromTexture(InMapInfo.MapImage);
	SoloMapDescription->SetText(InMapInfo.MapDescription);
	CurrentMapName->SetText(InMapInfo.MapName);
}

#pragma endregion

void UMainMenuWidget::MainMenuSetup()
{	
	SetIsFocusable(true);
	SetVisibility(ESlateVisibility::Visible);
	
	UWorld* World = GetWorld();

	if(World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();

		if(PlayerController) 
		{
			// Set input settings
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
}




