// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstances/OnlineShooterGameInstance.h"

void UOnlineShooterGameInstance::GoToMap(const FString& InMapURL)
{
	GetWorld()->ServerTravel(InMapURL);
}
