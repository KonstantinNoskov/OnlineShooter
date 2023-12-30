// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/CharacterOverlay.h"

#include "Components/TextBlock.h"

void UCharacterOverlay::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ElimText->SetVisibility(ESlateVisibility::Collapsed);
}
