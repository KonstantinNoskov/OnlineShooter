#include "Weapon/Shotgun.h"

#include "Characters/OnlineShooterCharacter.h"
#include "Components/LagCompensationComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlayerController/OnlineShooterPlayerController.h"
#include "Sound/SoundCue.h"

AShotgun::AShotgun()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AShotgun::BeginPlay()
{
	Super::BeginPlay();
}

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;
	AController* InstigatorController = OwnerPawn->Controller;

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		// The map stores all character's that were hit as a key and amount of hits as a value 
		TMap<AOnlineShooterCharacter*, uint32> HitMap;
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			// check if hit actor and Instigator controller is valid; also do server check 
			AOnlineShooterCharacter* OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(FireHit.GetActor());
			if (OnlineShooterCharacter)
			{
				if (HitMap.Contains(OnlineShooterCharacter))
				{
					HitMap[OnlineShooterCharacter]++;
				}

				else
				{
					HitMap.Emplace(OnlineShooterCharacter, 1);
				}

				// Play impact particles at the impact point
				if(ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(),
						ImpactParticles,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation() 
					);
				}

				// Play impact sound at the impact point
				if (ImpactSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
						this,
						ImpactSound,
						FireHit.ImpactPoint,
						.5f,
						FMath::FRandRange(-5.f, .5f)
					);
				}
			}
		}

		TArray<AOnlineShooterCharacter*> HitCharacters;
		
		for (auto HitPair : HitMap)
		{
			// check if hit actor and Instigator controller is valid; also do server check 
			if(HitPair.Key && InstigatorController )
			{
				if (HasAuthority() && !bUseServerSideRewind)
				{
					UGameplayStatics::ApplyDamage(
						HitPair.Key, // Character that was hit
						Damage * HitPair.Value, // Multiply Damage by number of times hit
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}
				
				HitCharacters.Add(HitPair.Key);
				}
			}

		if (!HasAuthority() && bUseServerSideRewind)
		{
			OnlineShooterOwnerCharacter = !OnlineShooterOwnerCharacter ? Cast<AOnlineShooterCharacter>(OwnerPawn) : OnlineShooterOwnerCharacter;
			OnlineShooterOwnerController = !OnlineShooterOwnerController ? Cast<AOnlineShooterPlayerController>(InstigatorController) : OnlineShooterOwnerController;
				
			if (OnlineShooterOwnerCharacter && OnlineShooterOwnerController && OnlineShooterOwnerCharacter->GetLagCompensation() && OnlineShooterOwnerCharacter->IsLocallyControlled())
			{
				OnlineShooterOwnerCharacter->GetLagCompensation()->ShotgunServerScoreRequest(
					HitCharacters,
					Start,
					HitTargets,
					OnlineShooterOwnerController->GetServerTime() - OnlineShooterOwnerController->GetSingleTripTime(),
					this
				);
			}
		}
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	// Get Muzzle flash socket
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(!MuzzleFlashSocket) return;

	// Get Muzzle flash socket location
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	// Get normalized vector from muzzle to aiming point
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();

	// Get location of scatter sphere 
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

	// Shoot random lines from scatter sphere center to perform scatter
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;
		
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();
		
		HitTargets.Add(ToEndLoc);
	}
}

void AShotgun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

