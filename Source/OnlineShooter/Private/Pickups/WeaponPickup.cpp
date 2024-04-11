﻿
#include "Pickups/WeaponPickup.h"

#include "Characters/OnlineShooterCharacter.h"
#include "Pickups/PickupSpawnPoint.h"


AWeaponPickup::AWeaponPickup()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AWeaponPickup::BeginPlay()
{
	Super::BeginPlay();

	if (SpawnPointOwner)
	{
		SpawnPointOwner->SetSpawnPointHighLight(true);
	}
	
}

void AWeaponPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AOnlineShooterCharacter* OverlapCharacter = Cast<AOnlineShooterCharacter>(OtherActor);
	if(OverlapCharacter)
	{
		Multicast_PlayEffect(OverlapCharacter);	
	}

	
}





