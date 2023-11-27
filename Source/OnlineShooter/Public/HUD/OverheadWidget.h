// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

class AOnlineShooterCharacter;
class UTextBlock;

/**
 * 
 */
UCLASS()
class ONLINESHOOTER_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	
	UPROPERTY(EditAnywhere, Category="Overhead Widget Properties", meta=(AllowPrivateAccess=true, Units="Seconds"))
	float GetPlayerNameTimeout = 30.f;
	
	UPROPERTY(EditAnywhere, Category="Overhead Widget Properties", meta=(AllowPrivateAccess=true, Units="Seconds"))
	float GetPlayerNameInterval = 0.1f;
	
	float TotalTime = -0.1f;
	
public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DisplayText;

	void SetDisplayText(FString TextToDisplay);
	
	UFUNCTION(BlueprintCallable)
	void ShowPlayerInfo(APawn* InPawn);

protected:

	virtual void NativeDestruct() override;
	
};
