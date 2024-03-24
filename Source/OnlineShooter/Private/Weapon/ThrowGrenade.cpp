#include "Weapon/ThrowGrenade.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

AThrowGrenade::AThrowGrenade()
{
	PrimaryActorTick.bCanEverTick = true;

	ThrowGrenadeParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>("ThrowGrenadeParticle");
	ThrowGrenadeParticleComponent->SetupAttachment(RootComponent);
}

void AThrowGrenade::BeginPlay()
{
	Super::BeginPlay();

	StartDestroyTimer(DestroyTime);
}
 
void AThrowGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AThrowGrenade::OnBounceGrenade(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
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

void AThrowGrenade::Destroyed()
{
	ExplodeDamage();
	AProjectile::Destroyed();
}

void AThrowGrenade::OnProjectileGrenadeStop(const FHitResult& ImpactResult)
{
	//
}

