// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/MainMenuGameMode.h"

#include "Characters/OnlineShooterCharacter.h"


AMainMenuGameMode::AMainMenuGameMode()
{
	DefaultPawnClass = AOnlineShooterCharacter::StaticClass();

	
}

