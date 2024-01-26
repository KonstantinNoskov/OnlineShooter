// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OnlineShooterPlayerController.generated.h"

class AOnlineShooterGameMode;
class UCharacterOverlay;
class AOnlineShooterHUD;


UCLASS()
class ONLINESHOOTER_API AOnlineShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;
	
protected:
	
	virtual void BeginPlay() override;
	
	UPROPERTY()
	float ClientServerDeltaTime = 0.f; // difference between client and server time

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	UPROPERTY()
	float TimeSyncRunningTime = 0.f;

private:

	UPROPERTY()
	AOnlineShooterGameMode* OnlineShooterGameMode;
	
	UPROPERTY()
	AOnlineShooterHUD* OnlineShooterHUD;
	
	UPROPERTY()
	float MatchTime = 0.f;

	UPROPERTY()
	float WarmupTime = 0.f;

	UPROPERTY()
	float LevelStartingTime = 0.f;

	UPROPERTY()
	float CooldownTime = 0.f;

	UPROPERTY()
	int32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	UPROPERTY()
	bool bInitializeCharacterOverlay = false;
	
	// Cached params to safely initialize HUD values 
	float HUDHealth;
	float HUDMaxHealth;
	float HUDShield;
	float HUDMaxShield;
	float HUDWeaponAmmo;
	float HUDCarriedAmmo;
	float HUDScore;
	int32 HUDDefeats;
	int32 HUDGrenades;

	bool bInitializeHealth = false;
	bool bInitializeShield = false;
	bool bInitializeWeaponAmmo = false;
	bool bInitializeCarriedAmmo = false;
	bool bInitializeScore = false;
	bool bInitializeDefeats = false;
	bool bInitializeGrenades = false;


#pragma region HIGH PING WARNING
	
	UPROPERTY()
	float HighPingRunningTime = 0.f;

	UPROPERTY()
	float PingAnimationRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;

#pragma endregion

protected:

	virtual float GetServerTime(); // Synced with server world clock
	virtual void ReceivedPlayer() override; // Sync with server clock as soon as possible

	UFUNCTION()
	void SetHUDTime();

	UFUNCTION()
	void HighPingWarning();

	UFUNCTION()
	void StopHighPingWarning();

	UFUNCTION()
	void CheckPing(float DeltaTime);

#pragma region SYNC SERVER/CLIENT TIME
	
	// Requests the current server time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void Server_RequestServerTime(float TimeOfClientRequest);

	// Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimerServerReceivedClientRequest);

	UFUNCTION()
	void CheckTimeSync(float DeltaTime);
	
	UFUNCTION()
	void PollInit();

	UFUNCTION(Server, Reliable)
	void Server_CheckMatchState();

	UFUNCTION(Client, Reliable)
	void Client_JoinMidgame(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown);

#pragma endregion
	
public:
	
	UFUNCTION()
	void SetHUDHealth(float Health, float MaxHealth);

	UFUNCTION()
	void SetHUDShield(float Shield, float MaxShield);

	UFUNCTION()
	void SetHUDScore(float Score);

	UFUNCTION()
	void SetHUDDefeats(int32 Defeats);

	UFUNCTION()
	void SetHUDElimMessage(FString ElimMessage);

	UFUNCTION()
	void HideElimMessage();

	UFUNCTION()
	void SetHUDWeaponAmmo(int32 WeaponAmmo);

	UFUNCTION()
	void SetHUDCarriedAmmo(int32 CarriedAmmo);

	UFUNCTION()
	void SetHUDMatchCountdown(float CountdownTime);

	UFUNCTION()
	void SetHUDAnnouncementCountdown(float CountdownTime);

	UFUNCTION()
	void SetHUDSniperScope(bool bIsAiming);

	UFUNCTION()
	void SetHUDGrenades(int32 Grenades);

	UFUNCTION()
	void OnMatchStateSet(FName State);

	UFUNCTION()
	void OnRep_MatchState();

	UFUNCTION()
	void HandleMatchHasStarted();

	UFUNCTION()
	void HandleCooldown();

	virtual void OnPossess(APawn* InPawn) override;
	
};
