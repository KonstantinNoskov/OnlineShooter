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
#include "PlayerController/OnlineShooterPlayerController.h"
#include "Weapon/Casing.h"


AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
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

	// Make Weapon pick up widget invisible by default
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}

	// Server logic
	if(HasAuthority())
	{
		// Set default collision settings
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

		// Bind overlap delegates
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Replication
void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState)
	DOREPLIFETIME(AWeapon, Ammo)
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

bool AWeapon::IsEmpty()
{
	return Ammo <= 0; 
}

bool AWeapon::IsFull()
{
	return Ammo == MagCapacity;
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if(WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

void AWeapon::OnRep_Ammo()
{
	OnlineShooterOwnerCharacter = !OnlineShooterOwnerCharacter ? Cast<AOnlineShooterCharacter>(GetOwner()) : OnlineShooterOwnerCharacter;
	if(OnlineShooterOwnerCharacter && OnlineShooterOwnerCharacter->GetCombatComponent() && IsFull())
	{
		OnlineShooterOwnerCharacter->GetCombatComponent()->JumpToShotgunEnd();
	}
	
	SetHUDAmmo();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if(!Owner)
	{
		OnlineShooterOwnerCharacter = nullptr;
		OnlineShooterOwnerController = nullptr;
	}

	else
	{
		SetHUDAmmo();
	}
	
}

// if player overlap weapon collision sphere, pass this weapon to player's @var OverlapWeapon
void AWeapon::OnSphereOverlap(UPrimitiveComponent* OvelappedComponent, AActor* OtherActor,
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
	switch (WeaponState) {

	case EWeaponState::EWS_Equipped:
	
		// Hide Equipped weapon pickup widget because we already have picked it
		ShowPickupWidget(false);
		
		// Disable EquippedWeapon overlap area collision
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);


		// Set physics for SMG strap
		if(WeaponType == EWeaponType::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		}

		// Disable Weapon mesh outline
		EnableCustomDepth(false);
		
		break;

	case EWeaponState::EWS_Dropped:

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
		
		break;
		
	default:
		break;
	}
}

void AWeapon::OnRep_WeaponState() 
{ 
	switch (WeaponState) {
	case EWeaponState::EWS_Initial:
		break;
		
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);

		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if(WeaponType == EWeaponType::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		}

		// Disable Weapon mesh outline
		EnableCustomDepth(false);
		
		break;
		
	case EWeaponState::EWS_Dropped:
		
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
		
		break;
		
	case EWeaponState::EWS_MAX:
		break;
		
	default:
		break;
	}
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

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
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



