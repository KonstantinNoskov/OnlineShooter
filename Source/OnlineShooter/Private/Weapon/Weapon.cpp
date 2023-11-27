#include "Weapon/Weapon.h"

// Replication
#include "Net/UnrealNetwork.h"

// References
#include "Characters/OnlineShooterCharacter.h"

// Components
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"


AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

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
		break;
		
	case EWeaponState::EWS_Dropped:
		break;
		
	case EWeaponState::EWS_MAX:
		break;
		
	default:
		break;
	}
}

// Show or hide weapon pick up widget
void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if(PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

