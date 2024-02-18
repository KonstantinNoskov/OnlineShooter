// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/MainMenuHUD.h"

#include "Blueprint/UserWidget.h"
#include "HUD/MainMenuWidget.h"

AMainMenuHUD::AMainMenuHUD()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMainMenuHUD::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void AMainMenuHUD::BeginPlay()
{
	Super::BeginPlay();

	AddMainMenu();
}


void AMainMenuHUD::AddMainMenu()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	
	if (PlayerController && MainMenuClass)
	{
		MainMenuOverlay = CreateWidget<UMainMenuWidget>(PlayerController, MainMenuClass);

		if (MainMenuOverlay)
		{
			MainMenuOverlay->MainMenuSetup();
			MainMenuOverlay->AddToViewport();	
		}
	}
}
