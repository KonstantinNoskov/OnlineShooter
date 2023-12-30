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
	
	virtual float GetServerTime(); // Synced with server world clock
	virtual void ReceivedPlayer() override; // Sync with server clock as soon as possible

	void SetHUDTime();


	// Sync time between client and server
	
	// Requests the current server time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void Server_RequestServerTime(float TimeOfClientRequest);

	// Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimerServerReceivedClientRequest);

	UFUNCTION()
	void CheckTimeSync(float DeltaTime);

	UPROPERTY()
	float ClinetServerDelta = 0.f; // difference between client and server time

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	UPROPERTY()
	float TimeSyncRunningTime = 0.f;
	
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
