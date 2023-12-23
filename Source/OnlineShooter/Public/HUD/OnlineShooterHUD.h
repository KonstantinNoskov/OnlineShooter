﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "OnlineShooterHUD.generated.h"

class UCharacterOverlay;

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
	
	UTexture2D* CrosshairCenter;
	UTexture2D* CrosshairLeft;
	UTexture2D* CrosshairRight;
	UTexture2D* CrosshairTop;
	UTexture2D* CrosshairBottom;

	float CrosshairSpread;
	FLinearColor CrosshairColor;
	
};

/**
 * 
 */
UCLASS()
class ONLINESHOOTER_API AOnlineShooterHUD : public AHUD
{
	GENERATED_BODY()

public:
	
	virtual void DrawHUD() override;

private:

	UPROPERTY()
	FHUDPackage HUDPackage;
	
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

protected:

	virtual void BeginPlay() override;

	void AddCharacterOverlay();
	
	
public:
	
	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<UUserWidget> CharacterOverlayClass;
	
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor = FLinearColor::White);
	
public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
