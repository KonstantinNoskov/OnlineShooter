// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OnlineShooterPlayerController.generated.h"

class AOnlineShooterHUD;

/**
 * 
 */
UCLASS()
class ONLINESHOOTER_API AOnlineShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	
	
protected:
	virtual void BeginPlay() override;

private:

	UPROPERTY()
	AOnlineShooterHUD* OnlineShooterHUD;

public:
	
	UFUNCTION()
	void SetHUDHealth(float Health, float MaxHealth);


	
};
