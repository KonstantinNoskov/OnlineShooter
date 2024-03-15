#include "Weapon/Weapon.h"

// Replication
#include "Net/UnrealNetwork.h"

// References
#include "Characters/OnlineShooterCharacter.h"
#include "Animation/AnimationAsset.h"

// Components
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlayerController/OnlineShooterPlayerController.h"
#include "Weapon/Casing.h"



AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
	
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::BeginPlay() 
{
	Super::BeginPlay();

	// Set default collision settings
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	// Bind overlap delegates
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);

	// Make Weapon pick up widget invisible by default
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (WeaponMesh && WeaponState == EWeaponState::EWS_Initial)
	{
		WeaponMesh->AddWorldRotation(FRotator(0.f, WeaponMeshTurnRate * DeltaTime, 0.f));
	}
}

// Replication
void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState)
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly)
}
bool AWeapon::IsMagEmpty()
{
	return Ammo <= 0; 
}
bool AWeapon::IsMagFull()
{
	return Ammo == MagCapacity;
}

// Defines pellets scatter 
FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(!MuzzleFlashSocket) return FVector();
	
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - TraceStart;

	// DEBUG
	if (bDebug)
	{
		// SCATTER
		if(bUseScatter)
		{
			DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
			DrawDebugSphere(GetWorld(), EndLoc, 20.f, 12, FColor::Orange, true);
			DrawDebugLine(
				GetWorld(),
				TraceStart,
				FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
				FColor::Cyan,
				true
					);	
		}	
	}
	
	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}
void AWeapon::EnableCustomDepth(bool bEnable)
{
	if(WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if(!Owner)
	{
		OnlineShooterOwnerCharacter = nullptr;
		OnlineShooterOwnerController = nullptr;
	}

	else if (WeaponState != EWeaponState::EWS_EquippedSecondary)
	{
		SetHUDAmmo();
	}
}

// if player overlap weapon collision sphere, pass this weapon to player's @var OverlapWeapon
void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AOnlineShooterCharacter* OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(OtherActor);
	if(OnlineShooterCharacter)
	{
		OnlineShooterCharacter->SetOverlappingWeapon(this);
	}
}


// if player stops overlap weapon collision sphere, set player's @var OverlapWeapon to null
void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AOnlineShooterCharacter* OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(OtherActor);
	if(OnlineShooterCharacter)
	{
		OnlineShooterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::SetWeaponState(EWeaponState NewState)
{
	WeaponState = NewState;
	OnWeaponStateSet();
}
void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
		case EWeaponState::EWS_Equipped:
			HandleWeaponEquipped();
			break;

		case EWeaponState::EWS_EquippedSecondary:
			HandleEquippedSecondary();
			break;
	
		case EWeaponState::EWS_Dropped:
			HandleWeaponDropped();
			break;
				
		default:
			break;
	}
}
void AWeapon::OnRep_WeaponState() 
{ 
	OnWeaponStateSet();
}

void AWeapon::HandleWeaponEquipped() 
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// Keep SMG strap physic when equipped
	if(WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	}

	// Disable Weapon mesh outline
	EnableCustomDepth(false);

	OnlineShooterOwnerCharacter = !OnlineShooterOwnerCharacter ? Cast<AOnlineShooterCharacter>(GetOwner()) : OnlineShooterOwnerCharacter;
	if(OnlineShooterOwnerCharacter && bUseServerSideRewind)
	{
		OnlineShooterOwnerController = !OnlineShooterOwnerController ? Cast<AOnlineShooterPlayerController>(OnlineShooterOwnerCharacter->Controller) : OnlineShooterOwnerController;
		if(OnlineShooterOwnerController && HasAuthority() && OnlineShooterOwnerController->HighPingDelegate.IsBound())
		{
			OnlineShooterOwnerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::HandleWeaponDropped() 
{
	if(HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); 
	
	// Enable Weapon mesh outline
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	OnlineShooterOwnerCharacter = !OnlineShooterOwnerCharacter ? Cast<AOnlineShooterCharacter>(GetOwner()) : OnlineShooterOwnerCharacter;
	if(OnlineShooterOwnerCharacter && bUseServerSideRewind)
	{
		OnlineShooterOwnerController = !OnlineShooterOwnerController ? Cast<AOnlineShooterPlayerController>(OnlineShooterOwnerCharacter->Controller) : OnlineShooterOwnerController;
		if(OnlineShooterOwnerController && HasAuthority() && OnlineShooterOwnerController->HighPingDelegate.IsBound())
		{
			OnlineShooterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::HandleEquippedSecondary()
{
	ShowPickupWidget(false);

	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// Keep SMG strap physic when equipped
	if(WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	}

	// Disable Weapon mesh outline
	EnableCustomDepth(false);

	OnlineShooterOwnerCharacter = !OnlineShooterOwnerCharacter ? Cast<AOnlineShooterCharacter>(GetOwner()) : OnlineShooterOwnerCharacter;
	if(OnlineShooterOwnerCharacter && bUseServerSideRewind)
	{
		OnlineShooterOwnerController = !OnlineShooterOwnerController ? Cast<AOnlineShooterPlayerController>(OnlineShooterOwnerCharacter->Controller) : OnlineShooterOwnerController;
		if(OnlineShooterOwnerController && HasAuthority() && OnlineShooterOwnerController->HighPingDelegate.IsBound())
		{
			OnlineShooterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewind = !bPingTooHigh; 
}

void AWeapon::Fire(const FVector& HitTarget)
{
	// Play fire animation
	if(FireAnimationSequence)
	{
		WeaponMesh->PlayAnimation(FireAnimationSequence, false);
	}
	
	// Casing valid check
	if(CasingClass)
	{
		// check if AmmoEjectSocket is valid
		const USkeletalMeshSocket* AmmoEjectSocket = GetWeaponMesh()->GetSocketByName(FName("AmmoEject"));
		if(AmmoEjectSocket)
		{
			// Get AmmoEjectSocket transform
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);

			// Create spawn params 
			FActorSpawnParameters SpawnParams;
			
			// check if world is valid
			UWorld* World = GetWorld();
			if(World)
			{
				// Spawn casing 
				World->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator(),
					SpawnParams
					);
			}
		}
	}

	// Decrement ammo and update HUD
	SpendRound();
}
void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
	Client_AddAmmo(AmmoToAdd);
}
void AWeapon::SetHUDAmmo()
{
	OnlineShooterOwnerCharacter = !OnlineShooterOwnerCharacter ? Cast<AOnlineShooterCharacter>(GetOwner()) : OnlineShooterOwnerCharacter;

	if(OnlineShooterOwnerCharacter)
	{
		OnlineShooterOwnerController = !OnlineShooterOwnerController ? Cast<AOnlineShooterPlayerController>(OnlineShooterOwnerCharacter->Controller) : OnlineShooterOwnerController;

		if(OnlineShooterOwnerController)
		{
			OnlineShooterOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::Client_AddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return;
	
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);

	OnlineShooterOwnerCharacter = !OnlineShooterOwnerCharacter ? Cast<AOnlineShooterCharacter>(GetOwner()) : OnlineShooterOwnerCharacter;
	if (OnlineShooterOwnerCharacter && OnlineShooterOwnerCharacter->GetCombatComponent() && IsMagFull())
	{
		OnlineShooterOwnerCharacter->GetCombatComponent()->JumpToShotgunEnd();
	}

	SetHUDAmmo();
}
void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo(); 

	if(HasAuthority())
	{
		Client_UpdateAmmo(Ammo);
	}
	else if (OnlineShooterOwnerCharacter && OnlineShooterOwnerCharacter->IsLocallyControlled())
	{
		++Sequence;
	}
}
void AWeapon::Client_UpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;
	
	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}
void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped); 
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);

	OnlineShooterOwnerCharacter = nullptr;
	OnlineShooterOwnerController = nullptr;
}

// Show or hide weapon pick up widget
void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if(PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}



