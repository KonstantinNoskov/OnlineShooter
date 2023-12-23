// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/OnlineShooterPlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "HUD/CharacterOverlay.h"
#include "HUD/OnlineShooterHUD.h"

void AOnlineShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	OnlineShooterHUD = Cast<AOnlineShooterHUD>(GetHUD());
}

void AOnlineShooterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;

	bool bHUDValid =
		OnlineShooterHUD &&
		OnlineShooterHUD->CharacterOverlay &&
		OnlineShooterHUD->CharacterOverlay->HealthBar &&
		OnlineShooterHUD->CharacterOverlay->HealthText;
	
	if(bHUDValid)
	{
		// Set Health bar Progress in percent 
		const float HealthPercent = Health / MaxHealth;
		OnlineShooterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);

		// Set Health Text
		FString HealthText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		OnlineShooterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}
