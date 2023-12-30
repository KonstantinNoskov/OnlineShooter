// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/OnlineShooterPlayerController.h"

#include "Characters/OnlineShooterCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "GameModes/OnlineShooterGameMode.h"
#include "HUD/Announcement.h"
#include "HUD/CharacterOverlay.h"
#include "HUD/OnlineShooterHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PlayerStates/OnlineShooterPlayerState.h"

void AOnlineShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	Server_CheckMatchState();
	
	OnlineShooterHUD = Cast<AOnlineShooterHUD>(GetHUD());
	Server_CheckMatchState();
		
}

void AOnlineShooterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SetHUDTime();

	CheckTimeSync(DeltaSeconds);

	PollInit();
}

void AOnlineShooterPlayerController::PollInit()
{
	if(!CharacterOverlay)
	{
		if(OnlineShooterHUD && OnlineShooterHUD->CharacterOverlay)
		{
			CharacterOverlay = OnlineShooterHUD->CharacterOverlay;
			if(CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
			}
		}
	}
}

void AOnlineShooterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOnlineShooterPlayerController, MatchState)
}

void AOnlineShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AOnlineShooterCharacter* OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(InPawn);

	if(OnlineShooterCharacter)
	{
		SetHUDHealth(OnlineShooterCharacter->GetHealth(), OnlineShooterCharacter->GetMaxHealth());
		HideElimMessage();
	}
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void AOnlineShooterPlayerController::SetHUDScore(float Score)
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;

	bool bHUDValid =
		OnlineShooterHUD &&
		OnlineShooterHUD->CharacterOverlay &&
		OnlineShooterHUD->CharacterOverlay->ScoreAmount &&
		OnlineShooterHUD->CharacterOverlay->ScoreText;

	if(bHUDValid)
	{
		// Set Score Amount
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score)); 
		OnlineShooterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}

	else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
	}
}

void AOnlineShooterPlayerController::SetHUDDefeats(int32 Defeats)
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;

	bool bHUDValid =
		OnlineShooterHUD &&
		OnlineShooterHUD->CharacterOverlay &&
		OnlineShooterHUD->CharacterOverlay->DefeatsAmount;

	if(bHUDValid)
	{
		// Set Defeats Amount
		FString DefeatText = FString::Printf(TEXT("%d"), Defeats); 
		OnlineShooterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatText));
	}
	
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats;
	}
}

void AOnlineShooterPlayerController::SetHUDElimMessage(FString ElimMessage)
{
	
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;

	bool bHUDValid =
		OnlineShooterHUD &&
		OnlineShooterHUD->CharacterOverlay &&
		OnlineShooterHUD->CharacterOverlay->ElimText;

	if(bHUDValid)
	{
		
		// Set Eliminitation message
		FString ElimText = FString::Printf(TEXT("Eliminated by %s"), *ElimMessage); 
		OnlineShooterHUD->CharacterOverlay->ElimText->SetText(FText::FromString(ElimText));

		OnlineShooterHUD->CharacterOverlay->ElimText->SetVisibility(ESlateVisibility::Visible);

		UE_LOG(LogTemp,Warning,TEXT("AOnlineShooterPlayerController::SetHUDElimMessage - SUCCESS: %s"), *OnlineShooterHUD->CharacterOverlay->ElimText->GetText().ToString())
		
	}
}

void AOnlineShooterPlayerController::HideElimMessage()
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;

	bool bHUDValid =
		OnlineShooterHUD &&
		OnlineShooterHUD->CharacterOverlay &&
		OnlineShooterHUD->CharacterOverlay->ElimText;

	if(bHUDValid)
	{
		OnlineShooterHUD->CharacterOverlay->ElimText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void AOnlineShooterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;

	bool bHUDValid =
		OnlineShooterHUD &&
		OnlineShooterHUD->CharacterOverlay &&
		OnlineShooterHUD->CharacterOverlay->WeaponAmmoAmount;

	if(bHUDValid)
	{
		FString WeaponAmmoText = FString::Printf(TEXT("%d"), Ammo); 
		OnlineShooterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(WeaponAmmoText));
	}
}

void AOnlineShooterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;

	bool bHUDValid =
		OnlineShooterHUD &&
		OnlineShooterHUD->CharacterOverlay &&
		OnlineShooterHUD->CharacterOverlay->CarriedAmmoAmount;

	if(bHUDValid)
	{
		FString CarriedAmmoText = FString::Printf(TEXT("%d"), Ammo); 
		OnlineShooterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText));
	}
}

void AOnlineShooterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;

	bool bHUDValid =
		OnlineShooterHUD &&
		OnlineShooterHUD->CharacterOverlay &&
		OnlineShooterHUD->CharacterOverlay->MatchCountdownText;

	if(bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		
		OnlineShooterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void AOnlineShooterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;

	bool bHUDValid =
		OnlineShooterHUD &&
		OnlineShooterHUD->Announcement &&
		OnlineShooterHUD->Announcement->WarmupTime;

	if(bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		
		OnlineShooterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void AOnlineShooterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;

	if(MatchState == MatchState::WaitingToStart)	TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress)	TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
		
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if(CountdownInt != SecondsLeft)
	{
		if(MatchState == MatchState::WaitingToStart)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}

		if(MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}
	
	CountdownInt = SecondsLeft;
}

#pragma region SYNC CLIENT/SERVER TIME

void AOnlineShooterPlayerController::Server_RequestServerTime_Implementation(float TimeOfClientRequest)
{
	// Get time on the server
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}
void AOnlineShooterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
	float TimerServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimerServerReceivedClientRequest + (.5f * RoundTripTime);

	ClinetServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}
float AOnlineShooterPlayerController::GetServerTime() 
{
	if(HasAuthority()) return GetWorld()->GetTimeSeconds();
	
	else return GetWorld()->GetTimeSeconds() + ClinetServerDelta;
}
void AOnlineShooterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		Server_RequestServerTime(GetWorld()->GetTimeSeconds());
	}
}
void AOnlineShooterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;

	if(IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		Server_RequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

#pragma endregion

void AOnlineShooterPlayerController::OnMatchStateSet(FName State)
{	
	MatchState = State;
	
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void AOnlineShooterPlayerController::OnRep_MatchState()
{
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void AOnlineShooterPlayerController::HandleMatchHasStarted() 
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;
	if(OnlineShooterHUD)
	{
		OnlineShooterHUD->AddCharacterOverlay();
		if(OnlineShooterHUD->Announcement)
		{
			OnlineShooterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AOnlineShooterPlayerController::Server_CheckMatchState_Implementation()  
{
	AOnlineShooterGameMode* GameMode = Cast<AOnlineShooterGameMode>(UGameplayStatics::GetGameMode(this));

	if(GameMode)
	{
		MatchState = GameMode->GetMatchState();
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		
		Client_JoinMidgame(MatchState, WarmupTime, MatchTime, LevelStartingTime);

		if(OnlineShooterHUD && MatchState == MatchState::WaitingToStart)
		{
			OnlineShooterHUD->AddAnnouncement();
		}
	}
}

void AOnlineShooterPlayerController::Client_JoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float StartingTime)
{
	MatchState = StateOfMatch;
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	OnMatchStateSet(MatchState);

	if(OnlineShooterHUD && MatchState == MatchState::WaitingToStart)
	{
		OnlineShooterHUD->AddAnnouncement();
	}
}





