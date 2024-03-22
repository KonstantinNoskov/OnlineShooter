﻿#include "PlayerController/OnlineShooterPlayerController.h"

#include "Characters/OnlineShooterCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "GameModes/OnlineShooterGameMode.h"
#include "GameStates/OnlineShooterGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PlayerStates/OnlineShooterPlayerState.h"

// HUD
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "HUD/Announcement.h"
#include "HUD/CharacterOverlay.h"
#include "HUD/Chat.h"
#include "HUD/OnlineShooterHUD.h"
#include "HUD/ReturnToMainMenu.h"
#include "HUD/SniperScopeWidget.h"
#include "OnlineShooter/Data/Announcement.h"

void AOnlineShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// Get HUD
	OnlineShooterHUD = Cast<AOnlineShooterHUD>(GetHUD());
	
	// Match State Check
	Server_CheckMatchState();
	
	// Add Controller Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(ControllerMappingContext, 1);
	}
}

void AOnlineShooterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Update Time
	SetHUDTime();
	
	// Sync with server
	CheckTimeSync(DeltaSeconds);

	// Make sure all HUD values are initialized correctly
	PollInit();

	// Check Ping
	CheckPing(DeltaSeconds);
}

void AOnlineShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!InputComponent) return;
	
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		// Toggle In-Game Menu
		EnhancedInputComponent->BindAction(MenuAction, ETriggerEvent::Started, this, &AOnlineShooterPlayerController::ShowReturnToMainMenu);

		// Toggle Chat
		EnhancedInputComponent->BindAction(ChatAction, ETriggerEvent::Started, this, &AOnlineShooterPlayerController::ToggleChat);
	}
}

void AOnlineShooterPlayerController::ShowReturnToMainMenu()
{
	// Menu subclass valid check
	if (!ReturnToMainMenuWidget) return;

	// Create Menu widget if necessary
	if (!ReturnToMainMenu)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidget);	
	}

	// Menu Widget valid check
	if (ReturnToMainMenu)	
	{
		// Toggle Menu Open/Close flag 
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;

		// Menu is open
		if (bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSetup();
		}

		// Menu is closed
		else
		{
			ReturnToMainMenu->MenuTearDown();
		}
	}
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
				if (bInitializeHealth)		SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeShield)		SetHUDShield(HUDShield, HUDMaxShield);
				if (bInitializeWeaponAmmo)	SetHUDWeaponAmmo(HUDWeaponAmmo);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeScore)		SetHUDScore(HUDScore);
				if (bInitializeDefeats)		SetHUDDefeats(HUDDefeats);
				
				AOnlineShooterCharacter* OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(GetPawn());
				if (OnlineShooterCharacter && OnlineShooterCharacter->GetCombatComponent())
				{
					if (bInitializeGrenades) SetHUDGrenades(OnlineShooterCharacter->GetCombatComponent()->GetGrenades());	
				}
			}
		}
	}
}
void AOnlineShooterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOnlineShooterPlayerController, MatchState)
	DOREPLIFETIME(AOnlineShooterPlayerController, bShowTeamScores)
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

#pragma region CHAT

void AOnlineShooterPlayerController::ToggleChat()
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;
	if (OnlineShooterHUD)
	{
		if (!OnlineShooterHUD->ChatWidget)
		{
			OnlineShooterHUD->AddChat();
			OnlineShooterHUD->ChatWidget->FocusChat();
			return;
		}	
		
		if (OnlineShooterHUD->ChatWidget->IsChatOpen())
		{
			OnlineShooterHUD->ChatWidget->SetVisibility(ESlateVisibility::Hidden);
			OnlineShooterHUD->ChatWidget->SetChatOpen(false);
		}

		else
		{
			OnlineShooterHUD->ChatWidget->SetVisibility(ESlateVisibility::Visible);
			OnlineShooterHUD->ChatWidget->SetChatOpen(true);
			OnlineShooterHUD->ChatWidget->ChatInput->SetFocus();
		}
	}
}


void AOnlineShooterPlayerController::BroadcastChatMessage(APlayerState* PublisherState, FString MessageText)
{
	ServerChatMessage(PublisherState, MessageText);
}

void AOnlineShooterPlayerController::ServerChatMessage_Implementation(APlayerState* PublisherState,
	const FString& MessageText)
{	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AOnlineShooterPlayerController* TempPlayerController = Cast<AOnlineShooterPlayerController>(*It);
		if (TempPlayerController)
		{
			TempPlayerController->MulticastChatMessage(PublisherState, MessageText);
		}
	}
}


void AOnlineShooterPlayerController::MulticastChatMessage_Implementation(APlayerState* PublisherState, const FString& MessageText)
{
	APlayerState* Self = GetPlayerState<APlayerState>();

	if (Self)
	{
		OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;
		if (OnlineShooterHUD)
		{
			OnlineShooterHUD->AddChat();

			if (OnlineShooterHUD->ChatWidget)
			{
				if (PublisherState == Self)
				{	
					OnlineShooterHUD->ChatWidget->AddChatMessageText("You", MessageText);
				}

				else
				{
					OnlineShooterHUD->ChatWidget->AddChatMessageText(PublisherState->GetPlayerName(), MessageText);
				}
			}
		}
	}
}

#pragma endregion

#pragma region GAINING THE LEAD

void AOnlineShooterPlayerController::BroadcastEliminate(APlayerState* Attacker, APlayerState* Victim)
{
	ClientEliminateAnnouncement(Attacker, Victim);
}

void AOnlineShooterPlayerController::ClientEliminateAnnouncement_Implementation(APlayerState* Attacker,
	APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	if (Attacker && Victim && Self)
	{
		OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;
		if (OnlineShooterHUD)
		{
			// You killed someone
			if (Attacker == Self && Victim != Self)
			{
				OnlineShooterHUD->AddEliminateAnnouncement("You", Victim->GetPlayerName());
				return;
			}

			// Someone killed you
			if (Victim == Self && Attacker != Self)
			{
				OnlineShooterHUD->AddEliminateAnnouncement(Attacker->GetPlayerName(), "you");
				return;
			}

			// You killed yourself
			if (Attacker == Victim && Attacker == Self)
			{
				OnlineShooterHUD->AddEliminateAnnouncement("You",  "yourself");
				return;
			}

			// Someone killed themselves
			if (Attacker == Victim && Attacker != Self)
			{
				OnlineShooterHUD->AddEliminateAnnouncement(Attacker->GetPlayerName(),  "themselves");
				return;
			}

			// Someone killed someone else
			OnlineShooterHUD->AddEliminateAnnouncement(Attacker->GetPlayerName(),  Victim->GetPlayerName());
		}
	}
}

#pragma endregion

#pragma region TEAMS

void AOnlineShooterPlayerController::HideTeamScores()
{
	bool bHUDValid =
		OnlineShooterHUD &&
		OnlineShooterHUD->CharacterOverlay &&
		OnlineShooterHUD->CharacterOverlay->RedTeamScore &&
		OnlineShooterHUD->CharacterOverlay->BlueTeamScore &&
		OnlineShooterHUD->CharacterOverlay->ScoreSpacerText;
		
	if(bHUDValid)
	{
		OnlineShooterHUD->CharacterOverlay->RedTeamScore->SetText(FText());
		OnlineShooterHUD->CharacterOverlay->BlueTeamScore->SetText(FText());
		OnlineShooterHUD->CharacterOverlay->ScoreSpacerText->SetText(FText());
	} 
}
void AOnlineShooterPlayerController::InitTeamScores()
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;
	
	bool bHUDValid =
		OnlineShooterHUD &&
		OnlineShooterHUD->CharacterOverlay &&
		OnlineShooterHUD->CharacterOverlay->RedTeamScore &&
		OnlineShooterHUD->CharacterOverlay->BlueTeamScore &&
		OnlineShooterHUD->CharacterOverlay->ScoreSpacerText;
		
	if(bHUDValid)
	{
		FString Zero("0");
		FString Spacer("-");

		
		OnlineShooterHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(Zero));
		OnlineShooterHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(Zero));
		OnlineShooterHUD->CharacterOverlay->ScoreSpacerText->SetText(FText::FromString(Spacer));
	} 
}
void AOnlineShooterPlayerController::SetHUDRedTeamScore(int32 RedScore)
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;
	
	bool bHUDValid =
		OnlineShooterHUD &&
		OnlineShooterHUD->CharacterOverlay &&
		OnlineShooterHUD->CharacterOverlay->RedTeamScore;
		
	if(bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), RedScore);
		OnlineShooterHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(ScoreText));
	} 
}
void AOnlineShooterPlayerController::SetHUDBlueTeamScore(int32 BlueScore)
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;
	
	bool bHUDValid =
		OnlineShooterHUD &&
		OnlineShooterHUD->CharacterOverlay &&
		OnlineShooterHUD->CharacterOverlay->BlueTeamScore;
		
	if(bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), BlueScore);
		OnlineShooterHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));
	} 
}
void AOnlineShooterPlayerController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}

FString AOnlineShooterPlayerController::GetInfoText(const TArray<AOnlineShooterPlayerState*>& Players)
{
	FString InfoTextString;
	
	AOnlineShooterPlayerState* OnlineShooterPlayerState = GetPlayerState<AOnlineShooterPlayerState>();
	if (!OnlineShooterPlayerState) return FString(); 
	
	if(!Players.Num())
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (Players.Num() == 1 && Players[0] == OnlineShooterPlayerState)
	{
		InfoTextString = Announcement::YouWon;
	}
	else if (Players.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("Winner: \n%s" ), *Players[0]->GetPlayerName());
	}
	else if (Players.Num() > 1)
	{	
		InfoTextString = Announcement::MultipleWinners;

		for (auto TiedPlayer : Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}
			
	OnlineShooterHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
	
	return InfoTextString;
}

FString AOnlineShooterPlayerController::GetTeamsInfoText(AOnlineShooterGameState* OnlineShooterGameState)
{
	if (!OnlineShooterGameState) return FString();
	FString InfoTextString;

	int32 RedTeamScore = OnlineShooterGameState->RedTeamScore;
	int32 BlueTeamScore = OnlineShooterGameState->BlueTeamScore;

	if (!RedTeamScore && !BlueTeamScore)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}

	else if (RedTeamScore == BlueTeamScore)
	{
		InfoTextString = Announcement::MultipleTeamsWinners;
		InfoTextString.Append(Announcement::RedTeam);
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(Announcement::BlueTeam);
		InfoTextString.Append(TEXT("\n"));
	}

	else if (RedTeamScore > BlueTeamScore)
	{
		InfoTextString = Announcement::RedTeamWin;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
	}

	else if (RedTeamScore < BlueTeamScore)
	{
		InfoTextString = Announcement::BlueTeamWin;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
	}
	
	return InfoTextString;
}


#pragma endregion


// Set Character Overlay
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
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}
void AOnlineShooterPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;

	bool bHUDValid =
		OnlineShooterHUD &&
		OnlineShooterHUD->CharacterOverlay &&
		OnlineShooterHUD->CharacterOverlay->ShieldBar &&
		OnlineShooterHUD->CharacterOverlay->ShieldText;
		
	if(bHUDValid)
	{
		// Set Health bar Progress in percent 
		const float ShieldPercent = Shield / MaxShield;
		OnlineShooterHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		
		// Set Health Text
		FString ShieldText = FString::Printf(TEXT("%d / %d"), FMath::FloorToInt(Shield), FMath::CeilToInt(MaxShield));
		OnlineShooterHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
		
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
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
		bInitializeScore = true;
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
		bInitializeDefeats = true;
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
void AOnlineShooterPlayerController::SetHUDWeaponAmmo(int32 WeaponAmmo)
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;

	bool bHUDValid =
		OnlineShooterHUD &&
		OnlineShooterHUD->CharacterOverlay &&
		OnlineShooterHUD->CharacterOverlay->WeaponAmmoAmount;

	if(bHUDValid)
	{
		FString WeaponAmmoText = FString::Printf(TEXT("%d"), WeaponAmmo); 
		OnlineShooterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(WeaponAmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = WeaponAmmo;
	}
}
void AOnlineShooterPlayerController::SetHUDCarriedAmmo(int32 CarriedAmmo)
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;

	bool bHUDValid =
		OnlineShooterHUD &&
		OnlineShooterHUD->CharacterOverlay &&
		OnlineShooterHUD->CharacterOverlay->CarriedAmmoAmount;

	if(bHUDValid)
	{
		FString CarriedAmmoText = FString::Printf(TEXT("%d"), CarriedAmmo); 
		OnlineShooterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = CarriedAmmo;
	}
}
void AOnlineShooterPlayerController::SetHUDGrenades(int32 Grenades)
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;

	bool bHUDValid =
		OnlineShooterHUD &&
		OnlineShooterHUD->CharacterOverlay &&
		OnlineShooterHUD->CharacterOverlay->GrenadesAmount; 

	if(bHUDValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades); 
		OnlineShooterHUD->CharacterOverlay->GrenadesAmount->SetText(FText::FromString(GrenadesText));
	}

	else
	{
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
}

// Set Match countdown widget
void AOnlineShooterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;

	bool bHUDValid =
		OnlineShooterHUD &&
		OnlineShooterHUD->CharacterOverlay &&
		OnlineShooterHUD->CharacterOverlay->MatchCountdownText;

	if(bHUDValid)
	{
		if(CountdownTime < 0.f)
		{
			OnlineShooterHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}
		
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
		if(CountdownTime < 0.f)
		{
			OnlineShooterHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		
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
	else if (MatchState == MatchState::Cooldown)	TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime; 
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	
	if(CountdownInt != SecondsLeft)
	{
		if(MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
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

// Set HighPingWarning
void AOnlineShooterPlayerController::HighPingWarning()
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;
	
	bool bHUDValid =
		OnlineShooterHUD &&
		OnlineShooterHUD->CharacterOverlay &&
		OnlineShooterHUD->CharacterOverlay->HighPingImage &&
		OnlineShooterHUD->CharacterOverlay->HighPingAnimation;
		
	if(bHUDValid)
	{
		OnlineShooterHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);

		UWidgetAnimation* HighPingAnimation = OnlineShooterHUD->CharacterOverlay->HighPingAnimation;
		OnlineShooterHUD->CharacterOverlay->PlayAnimation(HighPingAnimation, 0.f, 5);
	} 
}
void AOnlineShooterPlayerController::StopHighPingWarning()
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;

	bool bHUDValid =
		OnlineShooterHUD &&
		OnlineShooterHUD->CharacterOverlay &&
		OnlineShooterHUD->CharacterOverlay->HighPingImage &&
		OnlineShooterHUD->CharacterOverlay->HighPingAnimation;
	
	if(bHUDValid)
	{
		OnlineShooterHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);

		UWidgetAnimation* HighPingAnimation = OnlineShooterHUD->CharacterOverlay->HighPingAnimation;
		
		if(OnlineShooterHUD->CharacterOverlay->IsAnimationPlaying(HighPingAnimation))
		{
			OnlineShooterHUD->CharacterOverlay->StopAnimation(HighPingAnimation);
		}
	} 
}
void AOnlineShooterPlayerController::CheckPing(float DeltaTime) 
{
	if (HasAuthority()) return;
	
	HighPingRunningTime += DeltaTime;

	if(HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = !PlayerState ? GetPlayerState<APlayerState>() : PlayerState;

		if (PlayerState)
		{
			
			if (PlayerState->GetPingInMilliseconds() > HighPingThreshold) 
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}

		HighPingRunningTime = 0.f;
	}
	
	bool bHighPingAnimationPlaying =
		OnlineShooterHUD
		&& OnlineShooterHUD->CharacterOverlay
		&& OnlineShooterHUD->CharacterOverlay->HighPingAnimation
		&& OnlineShooterHUD->CharacterOverlay->IsAnimationPlaying(OnlineShooterHUD->CharacterOverlay->HighPingAnimation);

	if (bHighPingAnimationPlaying)
	{
		PingAnimationRunningTime += DeltaTime;

		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}
void AOnlineShooterPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

// Sniper scope
void AOnlineShooterPlayerController::SetHUDSniperScope(bool bIsAiming)
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;

	bool bHUDValid =
		OnlineShooterHUD &&
		OnlineShooterHUD->SniperScope &&
		OnlineShooterHUD->SniperScope->ScopeZoomIn;

	if(!OnlineShooterHUD->SniperScope)
	{
		OnlineShooterHUD->AddSniperScope();
	}
	
	if(bHUDValid)
	{
		if(bIsAiming)
		{
			OnlineShooterHUD->SniperScope->PlayAnimation(OnlineShooterHUD->SniperScope->ScopeZoomIn);
		}

		else
		{
			OnlineShooterHUD->SniperScope->PlayAnimation(
				OnlineShooterHUD->SniperScope->ScopeZoomIn,
				0.f,
				1,
				EUMGSequencePlayMode::Reverse
			);
		}
	}
}

#pragma region  SYNC CLIENT/SERVER TIME

void AOnlineShooterPlayerController::Server_RequestServerTime_Implementation(float TimeOfClientRequest)
{
	// Get time on the server
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();

	// Pass back to client ClientRequestTime and current server time
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}
void AOnlineShooterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimerServerReceivedClientRequest)
{
	// Getting delta between client request time and current client time. 
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;

	SingleTripTime = RoundTripTime * .5f;
	
	// Client gets current server time by summing server time we've just passed in and the client request time divided by 2. 
	float CurrentServerTime = TimerServerReceivedClientRequest + SingleTripTime;

	// Delta between current client and current server time
	ClientServerDeltaTime = CurrentServerTime - GetWorld()->GetTimeSeconds();
}
float AOnlineShooterPlayerController::GetServerTime() 
{
	// On Server we just getting it's current time we want to sync with.
	if(HasAuthority()) return GetWorld()->GetTimeSeconds();

	// Client should sync it's time with server by adding delta time which we've calculated earlier to client current time.
	return GetWorld()->GetTimeSeconds() + ClientServerDeltaTime; 
}

void AOnlineShooterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		// Pass Current client time to server
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

void AOnlineShooterPlayerController::OnMatchStateSet(FName State, bool bTeamsMatch)
{	
	MatchState = State;
	
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(true);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}
void AOnlineShooterPlayerController::OnRep_MatchState()
{
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}
void AOnlineShooterPlayerController::HandleMatchHasStarted(bool bTeamsMatch) 
{
	if (HasAuthority())
	{
		bShowTeamScores = bTeamsMatch;
	}
	
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;
	if(OnlineShooterHUD)
	{
		if(!OnlineShooterHUD->CharacterOverlay)
		{
			OnlineShooterHUD->AddCharacterOverlay();	
		}
		
		if(OnlineShooterHUD->Announcement)
		{
			OnlineShooterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}

		if (!HasAuthority()) return;
		
		if (bTeamsMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScores();
		}
	}
}
void AOnlineShooterPlayerController::HandleCooldown()
{
	OnlineShooterHUD = !OnlineShooterHUD ? Cast<AOnlineShooterHUD>(GetHUD()) : OnlineShooterHUD;
	if(OnlineShooterHUD)
	{
		OnlineShooterHUD->CharacterOverlay->RemoveFromParent();

	bool bHUDValid =
		OnlineShooterHUD->Announcement &&
		OnlineShooterHUD->Announcement->AnnouncementText &&
		OnlineShooterHUD->Announcement->InfoText;
		
		if(bHUDValid)
		{
			FString AnnouncementText = Announcement::NewMatchStartIn;
			OnlineShooterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			OnlineShooterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			
			AOnlineShooterGameState* OnlineShooterGameState = Cast<AOnlineShooterGameState>(UGameplayStatics::GetGameState(this));
			AOnlineShooterPlayerState* OnlineShooterPlayerState = GetPlayerState<AOnlineShooterPlayerState>();
			
			if(OnlineShooterGameState && OnlineShooterPlayerState)
			{
				TArray<AOnlineShooterPlayerState*> TopPlayers = OnlineShooterGameState->TopScoringPlayers;
				FString InfoTextStr = bShowTeamScores ? GetTeamsInfoText(OnlineShooterGameState) : GetInfoText(TopPlayers);
				
				OnlineShooterHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextStr));
			}
		}
	}

	AOnlineShooterCharacter* OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(GetPawn());

	if(OnlineShooterCharacter && OnlineShooterCharacter->GetCombatComponent())
	{
		OnlineShooterCharacter->bDisableGameplay = true;
		OnlineShooterCharacter->GetCombatComponent()->FireButtonPressed(false);
		OnlineShooterCharacter->GetCombatComponent()->SetAiming(false);
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
		CooldownTime = GameMode->CooldownTime;
		
		Client_JoinMidgame(MatchState, WarmupTime, MatchTime, LevelStartingTime, CooldownTime);
	}
}
void AOnlineShooterPlayerController::Client_JoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown) 
{
	OnMatchStateSet(MatchState);
	MatchState = StateOfMatch;
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	CooldownTime = Cooldown;

	if(OnlineShooterHUD && MatchState == MatchState::WaitingToStart)
	{
		OnlineShooterHUD->AddAnnouncement();
	}
}