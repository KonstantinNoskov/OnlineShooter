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

#if WITH_EDITOR

void AProjectileRocket::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileRocket, InitialSpeed))
	{
		if (RocketMovementComponent)
		{
			RocketMovementComponent->InitialSpeed = InitialSpeed;
			RocketMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}
 
#endif

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
	/*
	 * We don't want to destroy rocket object immediately because in that case smoke trail particle will be destroyed either.
	 * What we want here is to imitate rocket destruction by deactivating collision, hiding mesh, playing impact sound
	 * and particle on hit so the smoke trail particle had enough time to disappear smoothly.
	 * At the same time we start timer which is designed to actually destroy rocket after certain amount of time.
	 * Time to destroy defined by @var DestroyTime in Projectile.h. Also can be edit in blueprints (EditAnywhere).
	 */
	
	// Ignore owner
	if(OtherActor == GetOwner()) return;

	// Apply explode damage
	ExplodeDamage();

	// Launch Destroy timer
	StartDestroyTimer(3.f);

	// Show particles
	if(ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}

	// Play impact sound
	if(ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	// Hide mesh to imitate object destruction
	if(ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}

	// Deactivate smoke trail particle
	if (TrailSystemComponent)
	{
		TrailSystemComponent->Deactivate();
	}

	// Stop playing rocket flying sound
	if(ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}
	
	// Deactivate collision to prevent another rocket explosion if some actor hit it  
	if(CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AProjectileRocket::Destroyed()
{
	// We don't need to spawn an impact particles and sound from the parent class the second time, so we override this function to prevent it.
}





