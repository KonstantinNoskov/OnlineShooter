#include "PlayerStates/OnlineShooterPlayerState.h"

#include "Characters/OnlineShooterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/OnlineShooterPlayerController.h"
#include "Sound/SoundCue.h"


void AOnlineShooterPlayerState::BeginPlay()
{
	Super::BeginPlay();

	SetPlayerName(FString("Nagibator228"));
}

// Set Score amount
void AOnlineShooterPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);

	Character = !Character ? Cast<AOnlineShooterCharacter>(GetPawn()) : Character;

	if(Character)
	{
		Controller = !Controller ? Cast<AOnlineShooterPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

// Set Defeats amount
void AOnlineShooterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;

	Character = !Character ? Cast<AOnlineShooterCharacter>(GetPawn()) : Character;

	if(Character)
	{
		Controller = !Controller ? Cast<AOnlineShooterPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void AOnlineShooterPlayerState::Multicast_SetAttackerName_Implementation(const FString& Name)
{
	Character = !Character ? Cast<AOnlineShooterCharacter>(GetPawn()) : Character;

	if(Character)
	{
		Controller = !Controller ? Cast<AOnlineShooterPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->SetHUDElimMessage(Name);
		}
	}
}

void AOnlineShooterPlayerState::SetAttackerName(FString Name)
{
	Multicast_SetAttackerName(Name);
}

void AOnlineShooterPlayerState::MulticastPlayGainLeadAnnounce_Implementation()
{
	if (!GainLeadAnnounce) return;
	
	UGameplayStatics::PlaySound2D(this, GainLeadAnnounce);
}

void AOnlineShooterPlayerState::MulticastPlayLostLeadAnnounce_Implementation()
{
	if (!LostLeadAnnounce) return;
	
	UGameplayStatics::PlaySound2D(this, LostLeadAnnounce);
}


#pragma region REPLICATION

void AOnlineShooterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AOnlineShooterPlayerState, Defeats);
}

void AOnlineShooterPlayerState::OnRep_Defeats()
{
	Character = !Character ? Cast<AOnlineShooterCharacter>(GetPawn()) : Character;

	if(Character)
	{
		Controller = !Controller ? Cast<AOnlineShooterPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AOnlineShooterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	
	Character = !Character ? Cast<AOnlineShooterCharacter>(GetPawn()) : Character;

	if(Character)
	{
		Controller = !Controller ? Cast<AOnlineShooterPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

#pragma endregion



