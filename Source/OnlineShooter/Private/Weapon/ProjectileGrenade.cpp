#include "Weapon/ProjectileGrenade.h"

#include "Characters/OnlineShooterCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


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

	// Uncomment to destroy grenade in time. Otherwise grenade will be destroyed by reaching min velocity.
	//StartDestroyTimer(DestroyTime);
	
	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounceGrenade);
	ProjectileMovementComponent->OnProjectileStop.AddDynamic(this, &AProjectileGrenade::AProjectileGrenade::OnProjectileGrenadeStop);
}


void AProjectileGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectileGrenade::OnBounceGrenade(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	AOnlineShooterCharacter* OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(ImpactResult.GetActor());
	if (OnlineShooterCharacter && OnlineShooterCharacter != GetOwner())
	{
		// Apply explode damage
		ExplodeDamage();

		DestroyDelay(3.f);
	}
	
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

void AProjectileGrenade::OnProjectileGrenadeStop(const FHitResult& ImpactResult)
{
	// Apply explode damage
	ExplodeDamage();
	
	DestroyDelay(3.f);
}

void AProjectileGrenade::Destroyed()
{	
	// We don't need to spawn an impact particles and sound from the parent class the second time, so we override this function to prevent it.
}

