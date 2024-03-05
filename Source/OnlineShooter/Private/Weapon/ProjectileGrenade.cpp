#include "Weapon/ProjectileGrenade.h"

#include "Characters/OnlineShooterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Weapon/RocketMovementComponent.h"


AProjectileGrenade::AProjectileGrenade()
{
	PrimaryActorTick.bCanEverTick = false;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent); 
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement Component"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bShouldBounce = true;
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;

	//RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovement Component"));
	//RocketMovementComponent->bRotationFollowsVelocity = true;
	//RocketMovementComponent->SetIsReplicated(true);
	//RocketMovementComponent->bShouldBounce = true;
	
	ExplosionMinDamage = Damage;
}

#if WITH_EDITOR
void AProjectileGrenade::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	OnInitialSpeedEdit(PropertyChangedEvent);
	
}

void AProjectileGrenade::OnInitialSpeedEdit(const FPropertyChangedEvent& PropertyChangedEvent)
{
	 
	FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileGrenade, InitialSpeed))
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}

#endif

void AProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay();
	
	SpawnTrailSystem();
	StartDestroyTimer();
	
	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);

	//RocketMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);
}


void AProjectileGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	
	UE_LOG(LogTemp, Warning, TEXT("Bounce of: %s"), *ImpactResult.GetComponent()->GetName())
	if (BounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			BounceSound,
			GetActorLocation()
		);
	}
}

void AProjectileGrenade::Destroyed()
{
	ExplodeDamage();
	Super::Destroyed();
}

