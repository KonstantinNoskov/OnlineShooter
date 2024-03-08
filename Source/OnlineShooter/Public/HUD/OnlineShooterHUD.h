﻿#pragma once

#include "CoreMinimal.h"
#include "EliminateAnnouncement.h"
#include "GameFramework/HUD.h"
#include "OnlineShooterHUD.generated.h"

class AOnlineShooterPlayerController;
class USniperScopeWidget;
class UAnnouncement;
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


UCLASS()
class ONLINESHOOTER_API AOnlineShooterHUD : public AHUD
{
	GENERATED_BODY()

public:
	
	virtual void DrawHUD() override;

private:

	UPROPERTY()
	APlayerController* OwningPlayer;
	
	UPROPERTY()
	FHUDPackage HUDPackage;
	
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

protected:

	virtual void BeginPlay() override;

public:

	UPROPERTY(EditAnywhere, Category = "Widgets")
	TSubclassOf<UUserWidget> CharacterOverlayClass;
	
	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;
	
	UPROPERTY(EditAnywhere, Category = "Widgets")
	TSubclassOf<UUserWidget> AnnouncementClass;

	UPROPERTY()
	UAnnouncement* Announcement;

	UPROPERTY(EditAnywhere, Category = "Widgets")
	TSubclassOf<UUserWidget> SniperScopeClass;

	UPROPERTY()
	USniperScopeWidget* SniperScope;

	UFUNCTION()
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor = FLinearColor::White);

	UFUNCTION()
	void AddCharacterOverlay();

	UFUNCTION()
	void AddAnnouncement();

	UFUNCTION()
	void AddSniperScope();


#pragma region ELIMINATE ANNOUNCE

private:
	
	UPROPERTY(EditAnywhere, Category = "Widgets")
	TSubclassOf<UEliminateAnnouncement> EliminateAnnouncementClass;
	
	UEliminateAnnouncement* EliminateAnnouncement;

public:
	
	UFUNCTION()
	void AddEliminateAnnouncement(FString Attacker, FString Victim);
	
#pragma endregion
	
public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
}; 
