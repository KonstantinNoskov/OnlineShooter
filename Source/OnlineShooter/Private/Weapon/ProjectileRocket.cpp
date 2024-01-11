#include "Weapon/ProjectileRocket.h"

#include "GameFramework/Character.h"

// References
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"

// Add libs
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Weapon/RocketMovementComponent.h"


AProjectileRocket::AProjectileRocket()
{
	PrimaryActorTick.bCanEverTick = true;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("Rocket movement component"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	SpawnTrailSystem();

	if(!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit); 
	}

	if(ProjectileLoop && LoopingSoundAttenuation)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.f,
			1.f,
			0.f,
			LoopingSoundAttenuation,
			(USoundConcurrency*) nullptr,
			false
			);
	}
}

void AProjectileRocket::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{	
	if(OtherActor == GetOwner()) return;
	
	ExplodeDamage();

	StartDestroyTimer();

	if(ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}

	if(ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if(ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}
	
	if (TrailSystemComponent)
	{
		TrailSystemComponent->Deactivate();
	}

	if(ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}

	if(CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AProjectileRocket::Destroyed()
{
	// We don't need to spawn an impact particles and sound from the parent class the second time, so we override this function to prevent it.
}





