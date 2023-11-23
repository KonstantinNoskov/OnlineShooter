#pragma once

#include "Components/CombatComponent.h"

// References
#include "Characters/OnlineShooterCharacter.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/Weapon.h"

// Constructor
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

// Begin play
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

// Tick
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// Equip weapon
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (!Character || !WeaponToEquip) return;
	
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	// if had weapon socket...
	const USkeletalMeshSocket* WeaponHandSocket = Character->GetMesh()->GetSocketByName(FName("SKT_Weapon"));
	if (WeaponHandSocket)
	{
		// add equip weapon to a socket
		WeaponHandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}

	// Equipped weapon become owned by player that has picked it
	EquippedWeapon->SetOwner(Character);
}

