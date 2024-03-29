﻿#include "Weapon/Projectile.h"

// Components
#include "Components/BoxComponent.h"

// Kismet
#include "Kismet/GameplayStatics.h"

// Niagara
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

// References
#include "Sound/SoundCue.h"
#include "OnlineShooter.h"
#include "Characters/OnlineShooterCharacter.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);
		
	// Set collision
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECR_Block);
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	if(Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}
	
	if(HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectile::SpawnTrailSystem()
{
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
}

void AProjectile::ExplodeDamage()
{
	APawn* FiringPawn = GetInstigator();
	
	if (FiringPawn && HasAuthority())
	{
		if (AController* FiringController = FiringPawn->GetController())
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,						// World context object
				Damage,						// BaseDamage
		        ExplosionMinDamage,			// MinimumDamage
				GetActorLocation(),			// Origin
				ExplosionInnerRadius,		// DamageInnerRadius
				ExplosionOuterRadius,		// DamageOuterRadius
				1.f,						// Exponent damageFalloff 
				UDamageType::StaticClass(), // DamageTypeClass
				TArray<AActor*>(),			// IgnoredActors
				this,						// DamageCauser
				FiringController			// InstigatorController
			);

			// DEBUG
			if (bDebug)
			{
				DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionInnerRadius, 16, FColor::Green, false, 2.f);
				DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionOuterRadius, 16, FColor::Red, false, 2.f);	
			}
		}
	}
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                        FVector NormalImpulse, const FHitResult& Hit)
{
	Destroy();
}

void AProjectile::DestroyDelay(float Delay) 
{
	// Launch Destroy timer
	StartDestroyTimer(Delay);

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
		
	// Deactivate collision to prevent another rocket explosion if some actor hit it  
	if(CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AProjectile::StartDestroyTimer(float TimerDelay)
{
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectile::DestroyTimerFinished,
		TimerDelay
		);
}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}

void AProjectile::Destroyed()
{
	Super::Destroyed();

	if(ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}

	if(ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}



