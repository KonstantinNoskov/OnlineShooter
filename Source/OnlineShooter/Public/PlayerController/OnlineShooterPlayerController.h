// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OnlineShooterPlayerController.generated.h"

class AOnlineShooterHUD;


UCLASS()
class ONLINESHOOTER_API AOnlineShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void SetHUDTime();
	
private:

	UPROPERTY()
	AOnlineShooterHUD* OnlineShooterHUD;

	UPROPERTY()
	AOnlineShooterPlayerController* PlayerController;

	UPROPERTY()
	float MatchTime = 120.f;

	UPROPERTY()
	int32 CountdownInt = 0;

public:
	
	UFUNCTION()
	void SetHUDHealth(float Health, float MaxHealth);

	UFUNCTION()
	void SetHUDScore(float Score);

	UFUNCTION()
	void SetHUDDefeats(int32 Defeats);

	UFUNCTION()
	void SetHUDElimMessage(FString ElimMessage);

	UFUNCTION()
	void HideElimMessage();

	UFUNCTION()
	void SetHUDWeaponAmmo(int32 Ammo);

	UFUNCTION()
	void SetHUDCarriedAmmo(int32 Ammo);

	UFUNCTION()
	void SetHUDMatchCountdown(float CountdownTime);

	virtual void OnPossess(APawn* InPawn) override;


	
};
