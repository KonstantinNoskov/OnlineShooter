#include "Weapon/ProjectileRocket.h"

#include "GameFramework/Character.h"

// References
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"

// Add libs
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Weapon/RocketMovementComponent.h"


AProjectileRocket::AProjectileRocket()
{
	PrimaryActorTick.bCanEverTick = true;

	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("Rocket movement component"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if(TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
			);
	}

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
	
	
	APawn* FiringPawn = GetInstigator();
	
	if (FiringPawn && HasAuthority())
	{
		if (AController* FiringController = FiringPawn->GetController())
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,						// World context object
				Damage,						// BaseDamage
				MinDamage,					// MinimumDamage
				GetActorLocation(),			// Origin
				InnerRadius,				// DamageInnerRadius
				OuterRadius,				// DamageOuterRadius
				1.f,						// Exponent damageFalloff 
				UDamageType::StaticClass(), // DamageTypeClass
				TArray<AActor*>(),			// IgnoredActors
				this,						// DamageCauser
				FiringController			// InstigatorController
			);
			
			//DrawDebugSphere(GetWorld(), GetActorLocation(), InnerRadius, 16, FColor::Green, false, 2.f);
			//DrawDebugSphere(GetWorld(), GetActorLocation(), OuterRadius, 16, FColor::Red, false, 2.f);
		}
	}

	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectileRocket::DestroyTimerFinished,
		DestroyTime
		);

	if(ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}

	if(ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if(RocketMesh)
	{
		RocketMesh->SetVisibility(false);
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

void AProjectileRocket::DestroyTimerFinished()
{
	Destroy();
}

void AProjectileRocket::Destroyed()
{
	
}





