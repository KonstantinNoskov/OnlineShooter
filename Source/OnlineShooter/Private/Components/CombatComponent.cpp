#pragma once

#include "Components/CombatComponent.h"

// Components
#include "Characters/OnlineShooterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerController/OnlineShooterPlayerController.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"

// NET
#include "Net/UnrealNetwork.h"

// Sound
#include "Sound/SoundCue.h"

// Weapon
#include "Weapon/Weapon.h"
#include "Weapon/Shotgun.h"
#include "Weapon/Projectile.h"

// Kismet
#include "Kismet/GameplayStatics.h"

// Debug
#include "DrawDebugHelpers.h"

// HUD
#include "Components/BoxComponent.h"
#include "HUD/OnlineShooterHUD.h"
#include "Pickups/WeaponSpawnPoint.h"

// Constructor
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 800.f;
	AimingWalkSpeed = 400.f;
}

// Begin play
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}

		if(Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
		InitializeCarriedAmmo();
	
	}
}

// Tick
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;

		// Trace from the middle of viewport  
		TraceUnderCrosshair(HitResult);

		// ImpactPoint
		HitTarget = HitResult.ImpactPoint;

		// Draw crosshair
		SetHUDCrosshair(DeltaTime);

		// Aim zoom
		InterpFOV(DeltaTime);
	}

	FiringTimer(DeltaTime);
}

// Replication
void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming)
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);
	DOREPLIFETIME(UCombatComponent, bFiring);
	DOREPLIFETIME(UCombatComponent, FiringTime);
	
}

#pragma region EQUIP WEAPON

// Equip weapon
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (!Character || !WeaponToEquip) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	
	WeaponToEquip->GetWeaponMesh()->SetRelativeScale3D(FVector(1.f,1.f,1.f));

	AWeaponSpawnPoint* WeaponSpawnPoint = WeaponToEquip->GetSpawnPointOwner();
	if (WeaponSpawnPoint)
	{
		WeaponSpawnPoint->StartSpawnPickupTimer(WeaponToEquip);
		WeaponSpawnPoint->SetSpawnPointHighLight(true);
		WeaponToEquip->SetSpawnPointOwner(nullptr);
	}
	
	// if there's no secondary weapon
	if (EquippedWeapon && !SecondaryWeapon)
	{
		// put equipped weapon to a secondary slot
		EquipSecondaryWeapon(WeaponToEquip);	
	}
	else
	{
		EquipPrimaryWeapon(WeaponToEquip);	
	}
	
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
	//Character->StartingAimRotation = FRotator(0.f, Character->GetBaseAimRotation().Yaw, 0.f);
}
void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (!WeaponToEquip) return;

	if(SecondaryWeapon && SecondaryWeapon->GetWeaponType() == WeaponToEquip->GetWeaponType())
	{
		SecondaryWeapon->AddAmmo(WeaponToEquip->Ammo);
		CarriedAmmoMap[SecondaryWeapon->GetWeaponType()] +=  WeaponToEquip->Ammo;
		
		WeaponToEquip->Destroy();

		// Play equip weapon sound
		PlayEquippedWeaponSound(WeaponToEquip);
		
		return;
	}

	if(EquippedWeapon && EquippedWeapon->GetWeaponType() == WeaponToEquip->GetWeaponType())
	{
		EquippedWeapon->AddAmmo(WeaponToEquip->Ammo);
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] +=  WeaponToEquip->Ammo;
		UpdateCarriedAmmo();
		
		WeaponToEquip->Destroy();

		// Play equip weapon sound
		PlayEquippedWeaponSound(WeaponToEquip);
		
		return;
	}
	
	// Drop the current weapon if already have one
	//DropEquippedWeapon();

	if (EquippedWeapon) EquippedWeapon->Destroy();
	
	// Set weapon state to "Equipped"
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	// Attach equipped weapon to right hand
	AttachActorToRightHand(EquippedWeapon);

	// Set new owner for equipped weapon
	EquippedWeapon->SetOwner(Character);

	// Update HUD current ammo
	EquippedWeapon->SetHUDAmmo();

	// Update HUD carried ammo
	UpdateCarriedAmmo();

	// Play equip weapon sound
	PlayEquippedWeaponSound(WeaponToEquip);

	// Try to reload equipped weapon if it's empty
	ReloadEmptyWeapon();
}
void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (!WeaponToEquip) return;
	
	if (EquippedWeapon->GetWeaponType() == WeaponToEquip->GetWeaponType())
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] += WeaponToEquip->Ammo;
		UpdateCarriedAmmo();

		// Play equip weapon sound
		PlayEquippedWeaponSound(WeaponToEquip);
		
		WeaponToEquip->Destroy();
		
		return;
	}
	
	// Set weapon state to "Equipped"
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);

	// Attach picked up weapon to the Secondary Slot
	AttachActorToSecondarySlot(WeaponToEquip);
	
	// Play equip weapon sound
	PlayEquippedWeaponSound(WeaponToEquip);

	// Set new owner for equipped weapon
	SecondaryWeapon->SetOwner(Character);
	
}
void UCombatComponent::OnRep_EquippedWeapon()
{
	if(EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;

		AttachActorToRightHand(EquippedWeapon);
		PlayEquippedWeaponSound(EquippedWeapon);

		EquippedWeapon->SetHUDAmmo();
	}
}
void UCombatComponent::OnRep_SecondaryWeapon()
{
	if(SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		
		AttachActorToSecondarySlot(SecondaryWeapon);
		PlayEquippedWeaponSound(EquippedWeapon);
	}
}
void UCombatComponent::DropEquippedWeapon()
{
	if(EquippedWeapon)
	{
		EquippedWeapon->Dropped();	
	}
}
void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (!Character || !Character->GetMesh() || !ActorToAttach) return;
	
	// if weapon socket is valid...
	const USkeletalMeshSocket* WeaponHandSocket = Character->GetMesh()->GetSocketByName(FName("SKT_Weapon"));
	if (WeaponHandSocket)
	{
		// add equipped weapon to a socket
		WeaponHandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}
void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (!Character || !Character->GetMesh() || !ActorToAttach) return;
	
	// if weapon socket is valid...
	const USkeletalMeshSocket* WeaponHandSocket = Character->GetMesh()->GetSocketByName(FName("SKT_LeftHand"));
	if (WeaponHandSocket)
	{
		// add equipped weapon to a socket
		WeaponHandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}
void UCombatComponent::AttachActorToSecondarySlot(AActor* ActorToAttach)
{
	if (!Character || !Character->GetMesh() || !ActorToAttach) return;

	const USkeletalMeshSocket* SecondarySocket;
	
	switch (SecondaryWeapon->GetWeaponType()) {
	
		case EWeaponType::EWT_Pistol:
			SecondarySocket = Character->GetMesh()->GetSocketByName(FName("SKT_Holster"));
			break;

		case EWeaponType::EWT_RocketLauncher:
			SecondarySocket = Character->GetMesh()->GetSocketByName(FName("SKT_Pelvis"));
			break;
		
		default:
			SecondarySocket = Character->GetMesh()->GetSocketByName(FName("SKT_Back"));
			break;
	}
	
	if (SecondarySocket)
	{	
		// add equipped weapon to a secondary socket
		SecondarySocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}
void UCombatComponent::UpdateCarriedAmmo() 
{
	if (!EquippedWeapon) return;
	
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	
	Controller = !Controller ? Cast<AOnlineShooterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}
void UCombatComponent::PlayEquippedWeaponSound(AWeapon* WeaponToEquip)
{
	if(Character && WeaponToEquip && WeaponToEquip->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponToEquip->EquipSound, Character->GetActorLocation());
	}
}

#pragma endregion

// Pickup Ammo
void UCombatComponent::PickupAmmo(EWeaponType AmmoWeaponType, int32 AmmoAmount) 
{
	// Ammo type valid check
	if (CarriedAmmoMap.Contains(AmmoWeaponType))
	{
		// Add the pickup ammo to the carried ammo
		CarriedAmmoMap[AmmoWeaponType] = FMath::Clamp(CarriedAmmoMap[AmmoWeaponType] + AmmoAmount, 0, MaxCarriedAmmo);

		//CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		CarriedAmmo = CarriedAmmoMap[AmmoWeaponType];
		
		// Update carried ammo in HUD if still have ammo.
		Controller = !Controller ? Cast<AOnlineShooterPlayerController>(Character->Controller) : Controller;
		if(Controller && EquippedWeapon && EquippedWeapon->GetWeaponType() == AmmoWeaponType)
		{
			Controller->SetHUDCarriedAmmo(CarriedAmmo);	
		}
		
		// Reload immediately if out of ammo
		if(EquippedWeapon && EquippedWeapon->IsMagEmpty() && EquippedWeapon->GetWeaponType() == AmmoWeaponType)
		{
			Reload();
		}
	}
}

// Swap Weapon
bool UCombatComponent::ShouldSwapWeapon()
{
	return (EquippedWeapon && SecondaryWeapon);
}
void UCombatComponent::SwapWeapon() 
{
	if (bAiming)
	{
		SetAiming(false);	
	}
	
	if (!Character || CombatState != ECombatState::ECS_Unoccupied || !Character->HasAuthority()) return;

	CombatState = ECombatState::ECS_SwappingWeapon;
	Character->PlaySwapWeaponMontage();
 	Character->bFinishedSwapping = false;
}
void UCombatComponent::FinishSwapWeapon()
{
	if (Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}

	if (Character)
	{
		Character->bFinishedSwapping = true;
		//ReloadEmptyWeapon();
	}
}
void UCombatComponent::FinishSwapAttachWeapon()
{
	PlayEquippedWeaponSound(EquippedWeapon);

	if (!Character || !Character->HasAuthority()) return;
	
	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;
	
	// Set params as secondary weapon become primary
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);

	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	
	ReloadEmptyWeapon();
	
	// Set params as primary weapon become secondary 
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToSecondarySlot(SecondaryWeapon);
}

// Fire
void UCombatComponent::FireButtonPressed(bool bPressed)
{	
	bFireButtonPressed = bPressed;
	
	if (bFireButtonPressed)
	{
		Fire();
		bFiring = true;
	}
}
bool UCombatComponent::CanFire()
{
	if(!EquippedWeapon) return false;
	if(bLocallyReloading) return false;
	
	bool bShotgunReloading =
		!EquippedWeapon->IsMagEmpty() &&
		bCanFire &&
		CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun;

	if (bShotgunReloading) return true;
	
	return
	!EquippedWeapon->IsMagEmpty() &&
	bCanFire &&
	CombatState == ECombatState::ECS_Unoccupied;
}
void UCombatComponent::Fire() 
{
	if (CanFire())
	{
		bCanFire = false;
		FiringTime = FiringTimeThreshold;
		
		if(EquippedWeapon)
		{
			CrosshairShootFactor = .75f;
			
			switch (EquippedWeapon->FireType) {

				case EFireType::EFT_HitScan:
					FireHitScanWeapon();
				break;
				
				case EFireType::EFT_Projectile:
					FireProjectileWeapon();
				break;
				
				case EFireType::EFT_Shotgun:
					FireShotgunWeapon();
				break;
			
				default:
				break;
			}
		}
		
		StartFireTimer();
	}
}

void UCombatComponent::FiringTimer(float DeltaTime)
{
	if (bFiring)
	{
		FiringTime -= DeltaTime;

		if (FiringTime <= 0)
		{
			bFiring = false;
		}
	}
}

// Weapon type specific fire
void UCombatComponent::FireProjectileWeapon()
{
	if (EquippedWeapon && Character)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if (!Character->HasAuthority())
		{
			LocalFire(HitTarget);
		}
			
		Server_Fire(HitTarget);
	}
}
void UCombatComponent::FireHitScanWeapon()
{
	if (EquippedWeapon && Character)
	{	
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if (!Character->HasAuthority()) LocalFire(HitTarget);
		Server_Fire(HitTarget);
	}
}
void UCombatComponent::FireShotgunWeapon()
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	
	if (Shotgun && Character)
	{
		TArray<FVector_NetQuantize> HitTargets;
		Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);
		if (!Character->HasAuthority())
		{
			ShotgunLocalFire(HitTargets);
		}
		
		Server_ShotgunFire(HitTargets);
	}
}

// Local Fire
void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if(!EquippedWeapon) return;

	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}
void UCombatComponent::ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if(!Shotgun || !Character) return;
	
	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
	{
		bLocallyReloading = false;
		Character->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(TraceHitTargets);
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

// Common Fire RPC 
void UCombatComponent::Server_Fire_Implementation(const FVector_NetQuantize& TraceHitTarget) 
{
	bFiring = true;
	FiringTime = FiringTimeThreshold;
	Multicast_Fire(TraceHitTarget);
}
void UCombatComponent::Multicast_Fire_Implementation(const FVector_NetQuantize& TraceHitTarget) 
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalFire(TraceHitTarget);
}

// Shotgun Fire RPC
void UCombatComponent::Server_ShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	Multicast_ShotgunFire(TraceHitTargets);
}
void UCombatComponent::Multicast_ShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	ShotgunLocalFire(TraceHitTargets);
}

void UCombatComponent::StartFireTimer()
{
	if (!EquippedWeapon || !Character) return;

	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &UCombatComponent::OnFireTimerFinished, EquippedWeapon->GetFireRate());
}
void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
}
void UCombatComponent::OnFireTimerFinished()
{
	if(!EquippedWeapon) return;

	bCanFire = true;
	
	if(bFireButtonPressed && EquippedWeapon->IsAutomatic()) 
	{
		Fire();
	}
	
	ReloadEmptyWeapon();
}
void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = !Controller ? Cast<AOnlineShooterPlayerController>(Character->Controller) : Controller;
	if(Controller && EquippedWeapon)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	bool bJumpToShotgunEnd =
		CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&
		CarriedAmmo == 0;

	if (bJumpToShotgunEnd)
	{
		JumpToShotgunEnd();
	}
}
void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState) {
		
		case ECombatState::ECS_Unoccupied:
			if(bFireButtonPressed)
			{
				Fire();
			}
			break;
		
		case ECombatState::ECS_Reloading:
			
			if (Character && !Character->IsLocallyControlled())
			{
				HandleReload();	
			}
			break;

		case ECombatState::ECS_ThrowingGrenade:
			if (Character && !Character->IsLocallyControlled())	
			{
				Character->PlayThrowGrenadeMontage(); // Play throwing grenade animation
				ShowAttachedGrenade(true); // Show the attached grenade mesh
			}
			break;

		case ECombatState::ECS_SwappingWeapon:
			if (Character && !Character->IsLocallyControlled())
			{
				Character->PlaySwapWeaponMontage();
			}
			break;
		
		default:
			break;
	}
}

// Reload
void UCombatComponent::Reload()
{
	bool bCanReload =
		CarriedAmmo > 0
	&& CombatState == ECombatState::ECS_Unoccupied
	&& EquippedWeapon
	&& !EquippedWeapon->IsMagFull()
	&& Character
	&& !Character->IsEliminated()
	&& !bLocallyReloading;

	
 	if (bCanReload)
	{
		Server_Reload();
		HandleReload();

		SetAiming(false);
 		
 		bLocallyReloading = true;
	}
	
}
void UCombatComponent::Server_Reload_Implementation()
{
	if (!Character || !EquippedWeapon) return;
	
	CombatState = ECombatState::ECS_Reloading;
	
	if (!Character->IsLocallyControlled())
	{
		HandleReload();
	}
}
void UCombatComponent::ReloadEmptyWeapon()
{
	if(EquippedWeapon && EquippedWeapon->IsMagEmpty())
	{
		Reload();
	}
}
void UCombatComponent::FinishReloading()
{
	if(!Character) return;

	bLocallyReloading = false;
	
	if(Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	
	if(bFireButtonPressed)
	{
		Fire();
	}
}
int32 UCombatComponent::AmountToReload()
{
	if(!EquippedWeapon) return 0;

	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);

		return FMath::Clamp(RoomInMag, 0, Least); 
	}

	return 0;
}
void UCombatComponent::HandleReload() 
{
	if (Character)
	{	
		Character->PlayReloadMontage();
	}
}
void UCombatComponent::UpdateAmmoValues()
{
	if(!Character || !EquippedWeapon) return;
	
	int32 ReloadAmount = AmountToReload();
	
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	
	Controller = !Controller ? Cast<AOnlineShooterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	
	EquippedWeapon->AddAmmo(ReloadAmount);	
}
void UCombatComponent::UpdateShotgunAmmoValues()
{
	if(!Character || !EquippedWeapon) return;

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = !Controller ? Cast<AOnlineShooterPlayerController>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	EquippedWeapon->AddAmmo(1);
	bCanFire = true;

	if(EquippedWeapon->IsMagFull() || CarriedAmmo == 0)
	{
		JumpToShotgunEnd();
	}
}
void UCombatComponent::ShotgunShellReload()
{
	if(Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues();	
	}
}
void UCombatComponent::JumpToShotgunEnd()
{
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	
	if(AnimInstance && Character->GetReloadMontage())
	{
		FName SectionName = FName("ShotgunEnd");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

// Throw Grenade
void UCombatComponent::ThrowGrenade() 
{
	// Check for ability to throw grenade
	bool bNotAbleToThrow =
		CombatState != ECombatState::ECS_Unoccupied ||
		!EquippedWeapon ||
		!Grenades ||
		Character->IsEliminated();
	
	if (bNotAbleToThrow) return;
	
	// Set combat state
	CombatState = ECombatState::ECS_ThrowingGrenade;
	
	if (Character)
	{
		Character->PlayThrowGrenadeMontage(); // Play throwing grenade animation
		AttachActorToLeftHand(EquippedWeapon); // Attach equipped weapon to the left hand.
		ShowAttachedGrenade(true); // Show the attached grenade mesh
	}

	// Call on server for throw grenade
	if (Character && !Character->HasAuthority())
	{
		Server_ThrowGrenade();	
	}

	// Local Call
	if (Character && Character->HasAuthority())
	{
		// Decrement grenades amount
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);

		// Update HUD
		UpdateHUDGrenades();
	}
}
void UCombatComponent::Server_ThrowGrenade_Implementation() 
{
	// Grenades check
	if (!Grenades) return;
	
	// Set combat state
	CombatState = ECombatState::ECS_ThrowingGrenade;

	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	
	// Decrement grenades amount
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);

	// Update HUD
	UpdateHUDGrenades(); 
}
void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}
void UCombatComponent::ThrowGrenadeFinished() 
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}
void UCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);
	
	if(Character && Character->IsLocallyControlled())
	{
		Server_LaunchGrenade(HitTarget);	
	}
}
void UCombatComponent::Server_LaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if(Character && GrenadeClass && Character->GetAttachedGrenade())
	{
		// Set Spawn point
		const FVector GrenadeSpawnLocation = Character->GetAttachedGrenade()->GetComponentLocation();

		// Set Direction
		FVector ToTarget = Target - GrenadeSpawnLocation;

		// Set spawn params
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		
		// Spawn grenade
		UWorld* World = GetWorld();
		if (World)
		{
			AProjectile* SpawnedProjectile = World->SpawnActor<AProjectile>(
				GrenadeClass,
				GrenadeSpawnLocation,
				ToTarget.Rotation(),
				SpawnParams
				);

			if (SpawnedProjectile && SpawnedProjectile->CollisionBox)
			{
				SpawnedProjectile->CollisionBox->IgnoreActorWhenMoving(Character, true);	
			}
		}
	}
}
void UCombatComponent::UpdateHUDGrenades()
{
	Controller = !Controller ? Cast<AOnlineShooterPlayerController>(Character->Controller) : Controller;

	if (Controller)
	{
		Controller->SetHUDGrenades(Grenades);
	}
}
void UCombatComponent::OnRep_Grenades()
{
	// Update HUD
	UpdateHUDGrenades();
}

// Crosshair & Aiming
void UCombatComponent::SetAiming(bool bIsAiming)
{
	if(!Character || !EquippedWeapon || Character->GetCombatState() == ECombatState::ECS_ThrowingGrenade || Character->GetCombatState() == ECombatState::ECS_SwappingWeapon ) return;
	
	bAiming = bIsAiming;
	Server_SetAiming(bIsAiming); 
	
	// Calculate walk speed aiming factor
	Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimingWalkSpeed : BaseWalkSpeed;
	
	if(Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Controller = !Controller ? Cast<AOnlineShooterPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDSniperScope(bIsAiming);

			if(bIsAiming && EquippedWeapon->ZoomInSound)
			{
				UGameplayStatics::PlaySound2D(this, EquippedWeapon->ZoomInSound);
			}

			else if (EquippedWeapon->ZoomOutSound)
			{
				UGameplayStatics::PlaySound2D(this, EquippedWeapon->ZoomOutSound);
			}
		}
	}
	
	if (Character->IsLocallyControlled())
	{
		bAimingButtonPressed = bIsAiming;
	}
}
void UCombatComponent::OnRep_Aiming()
{
	if (Character && Character->IsLocallyControlled())
	{
		bAiming = bAimingButtonPressed;
	}
}
void UCombatComponent::Server_SetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimingWalkSpeed : BaseWalkSpeed;
}
void UCombatComponent::TraceUnderCrosshair(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Crosshair 2D position on viewport
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	// Storage for Crosshair 3D position and direction
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Result of crosshair deprojection
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);
	
	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}
		
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		// Shoot crosshair line trace
		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECC_Visibility);

		// if line trace didn't hit anything then impact point would be equal to TRACE_LENGTH
		if(!TraceHitResult.bBlockingHit) 
		{
			TraceHitResult.ImpactPoint = End;
		}
		else // otherwise show hit point
		{
			//DrawDebugSphere(GetWorld(),TraceHitResult.ImpactPoint, 12.f, 12, FColor::Red);
		}

		// if line trace hit another player we want to change crosshair color
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
		}
		
	}
}
void UCombatComponent::SetHUDCrosshair(float DeltaTime)
{
	if(!Character || !Character->Controller) return;

	Controller = !Controller ? Cast<AOnlineShooterPlayerController>(Character->Controller) : Controller;

	if(Controller)
	{
		HUD = !HUD ? Cast<AOnlineShooterHUD>(Controller->GetHUD()) : HUD;

		if(HUD)
		{
			if (EquippedWeapon && EquippedWeapon->GetWeaponType() != EWeaponType::EWT_SniperRifle)
			{
				HUDPackage.CrosshairCenter	= EquippedWeapon->CrosshairCenter;
				HUDPackage.CrosshairLeft	= EquippedWeapon->CrosshairLeft;
				HUDPackage.CrosshairRight	= EquippedWeapon->CrosshairRight;
				HUDPackage.CrosshairTop		= EquippedWeapon->CrosshairTop;
				HUDPackage.CrosshairBottom	= EquippedWeapon->CrosshairBottom;
			}
			else
			{
				HUDPackage.CrosshairCenter	= nullptr;
				HUDPackage.CrosshairLeft	= nullptr;
				HUDPackage.CrosshairRight	= nullptr;
				HUDPackage.CrosshairTop		= nullptr;
				HUDPackage.CrosshairBottom	= nullptr;
			}
			
			// Calculate crosshair spread
			FVector2d WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			
			FVector2d VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			// Velocity Factor
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			// InAir Factor
			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			// Aim Factor
			if(bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, .58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			// Shoot Factor
			CrosshairShootFactor = FMath::FInterpTo(CrosshairShootFactor, 0.f, DeltaTime, 5.f);
			
			HUDPackage.CrosshairSpread =
				.5f +
				CrosshairVelocityFactor +
				CrosshairInAirFactor -
				CrosshairAimFactor +
				CrosshairShootFactor;
			
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}
void UCombatComponent::InterpFOV(float DeltaTime)
{
	if(!EquippedWeapon) return;

	if(bAiming)
	{	
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	if (Character && Character->GetFollowCamera())
	{
		 Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}