// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/Solo/SoloMapButton.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "HUD/MainMenuWidget.h"

void USoloMapButton::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Bind Delegates
	if (MapButton)	{ MapButton->OnClicked.AddDynamic(this, &ThisClass::SoloMapButtonOnClicked); }
	if (MapButton)	{ MapButton->OnHovered.AddDynamic(this, &ThisClass::SoloMapButtonOnHovered); }
	
	// Setup
	if (MapButtonText) MapButtonText->SetText(Info.MapName);
}

void USoloMapButton::SoloMapButtonOnClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("CLICK"))
	UE_LOG(LogTemp, Warning, TEXT("URL: %s\nMapName: %s\nDescription: %s"), *Info.MapURL, *Info.MapName.ToString(), *Info.MapDescription.ToString())

	TravelToMap(Info.MapURL);
}

void USoloMapButton::TravelToMap(const FString& InMapURL) const
{
	UOnlineShooterGameInstance* GameInstance = Cast<UOnlineShooterGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		GameInstance->GoToMap(InMapURL);
		
		if (APlayerController* Controller = GetWorld()->GetFirstPlayerController())
		{
			Controller->SetShowMouseCursor(false);

			FInputModeGameOnly InputData;
			Controller->SetInputMode(InputData);
		}
	}
}

void USoloMapButton::SoloMapButtonOnHovered()
{
	if (ParentWidget)
	{
		ParentWidget->UpdateCurrentMapInfo(Info); 
	}
}
