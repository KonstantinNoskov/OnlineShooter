// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/OverheadWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"


void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerInfo(APawn* InPawn)
{
	ENetRole RemoteRole = InPawn->GetRemoteRole();
	FString Role;

	switch (RemoteRole) {
	case ROLE_None:
		Role = FString("None");
		break;
	case ROLE_SimulatedProxy:
		Role = FString("Simulated Proxy");
		break;
	case ROLE_AutonomousProxy:
		Role = FString("Autonomous Proxy");
		break;
	case ROLE_Authority:
		Role = FString("Authority");
		break;
	
	default:
		break;
	}

	
	FString PlayerName;
	
	/* Trying to get PlayerState
	 * ===========================================================================================================
	 *
	 * This timer needs to make sure that we get info about clients.
	 *
	 * This logic will be executed till we get player state or till the time is out (TotalTime)
	 *
	 * @var TotalTime			  - Time limit before end up trying to get player state.
	 * @var GetPlayerNameInterval - Timeout between tries
	 *
	 * ===========================================================================================================
	 */
	
	// Try to get reference to player state
	const APlayerState* PlayerState = InPawn->GetPlayerState();

	// if somehow we don't have a player state pointer 
	if(!PlayerState || !*PlayerState->GetPlayerName() && TotalTime < GetPlayerNameTimeout)
	{
		// Create Timer
		FTimerHandle GetPlayerStateTimer;

		// Bind our DisplayPlayerInfo function to it 
		FTimerDelegate TryAgainDelegate;
		TryAgainDelegate.BindUFunction(this, FName("ShowPlayerInfo"), InPawn);

		// Set Timer
		GetWorld()->GetTimerManager().SetTimer(GetPlayerStateTimer, TryAgainDelegate, GetPlayerNameInterval, false, 0.1f);
		TotalTime += GetPlayerNameInterval;
		
		return;
	}

	// If eventually we get a PlayerState, get PlayerName from it
	if (PlayerState) PlayerName = PlayerState->GetPlayerName();
	
	// Display all player info we want
	FString PlayerInfoAsString = FString::Printf(TEXT("%s"), *PlayerName);
	SetDisplayText(PlayerInfoAsString);
}

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();
	
	Super::NativeDestruct();
}
