// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileWeapon.h"

#include "Components/BoxComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/Projectile.h"


AProjectileWeapon::AProjectileWeapon()
{
}

void AProjectileWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}
void AProjectileWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	UWorld* World = GetWorld();
	
	if(MuzzleFlashSocket && World)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());

		// From muzzle flash socket to hit location from TraceUnderCrosshair
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();

		FActorSpawnParameters SpawnParams;

		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;
		
		AProjectile* SpawnedProjectile = nullptr;
		if (bUseServerSideRewind)
		{
			// Server 
			if (InstigatorPawn->HasAuthority()) 
			{
				// Locally controlled - DO NOT spawn server-side rewind projectile. Spawn replicated projectile instead.
				// No SSR
				if (InstigatorPawn->IsLocallyControlled())
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation,SpawnParams);

					SpawnedProjectile->bUserServerSideRewind = false;
					SpawnedProjectile->Damage = Damage;
					SpawnedProjectile->CritDamage = Damage * CritFactor;

					if (SpawnedProjectile && SpawnedProjectile->CollisionBox)
					{
						SpawnedProjectile->CollisionBox->IgnoreActorWhenMoving(Owner, true);
					}
				}

				// Not locally controlled - spawn non-replicated projectile, no SSR
				else
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation,SpawnParams);
					SpawnedProjectile->bUserServerSideRewind = true;
				}
			}

			// Client, using SSR
			else
			{
				if (InstigatorPawn->IsLocallyControlled()) // client, locally controlled - spawn non-replicated projectile, use SSR
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation,SpawnParams);
					SpawnedProjectile->bUserServerSideRewind = true;
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
				}

				// client, not locally controlled - spawn non-replicated projectile, no SSR
				else
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation,SpawnParams);
					SpawnedProjectile->bUserServerSideRewind = false;
				}
			}
		}

		// Weapon not using SSR
		else
		{
			if (InstigatorPawn->HasAuthority())
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation,SpawnParams);
				SpawnedProjectile->bUserServerSideRewind = false;
				SpawnedProjectile->Damage = Damage;
				SpawnedProjectile->CritDamage = Damage * CritFactor;
				SpawnedProjectile->CollisionBox->IgnoreActorWhenMoving(SpawnParams.Owner, true);
			}
		}
	}
}

