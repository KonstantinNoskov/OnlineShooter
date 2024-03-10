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

void AShotgun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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

		// Body HitMap will keep track about all body hits
		TMap<AOnlineShooterCharacter*, uint32> BodyHitMap;

		// Headshot HitMap will keep track about all headshot hits
		TMap<AOnlineShooterCharacter*, uint32> HeadshotHitMap;

		// DamageMap will accumulate all damage from body and head combined
		TMap<AOnlineShooterCharacter*, float> DamageMap;

		// Stores pointer to all Hit characters (Used for Server-Side Rewind logic ONLY)
		TArray<AOnlineShooterCharacter*> HitCharacters;

		
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			// check if hit actor is valid
			AOnlineShooterCharacter* OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(FireHit.GetActor());
			if (OnlineShooterCharacter)
			{
				
				// Fill the Headshot HitMap
				const bool bHeadshot = FireHit.BoneName.ToString() == FString("head");
				if (bHeadshot)
				{
					if (HeadshotHitMap.Contains(OnlineShooterCharacter)) HeadshotHitMap[OnlineShooterCharacter]++;
					else HeadshotHitMap.Emplace(OnlineShooterCharacter, 1);	
				}

				else
				{
					// Fill the Body HitMap
					if (BodyHitMap.Contains(OnlineShooterCharacter)) BodyHitMap[OnlineShooterCharacter]++;
					else BodyHitMap.Emplace(OnlineShooterCharacter, 1);	
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
		
		// Accumulate body damage by multiplying times Hit x Damage - store in DamageMap
		for (auto BodyHitPair : BodyHitMap)
		{
			if(BodyHitPair.Key)
			{
				DamageMap.Emplace(BodyHitPair.Key, BodyHitPair.Value * Damage);
				HitCharacters.AddUnique(BodyHitPair.Key);
			}
		}

		// Accumulate Headshot damage by multiplying times Hit x Damage - store in DamageMap
		for (auto HeadshotHitPair : HeadshotHitMap)
		{
			if(HeadshotHitPair.Key)
			{
				if (DamageMap.Contains(HeadshotHitPair.Key)) DamageMap[HeadshotHitPair.Key] += HeadshotHitPair.Value * CritDamage;
				else DamageMap.Emplace(HeadshotHitPair.Key, HeadshotHitPair.Value * CritDamage);	
				
				HitCharacters.AddUnique(HeadshotHitPair.Key);
			}
		}

		// Apply total damage from body and head hits combined
		for (auto DamagePair : DamageMap)
		{
			if (DamagePair.Key && InstigatorController)
			{
				bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
				if (HasAuthority() && bCauseAuthDamage)
				{
					UGameplayStatics::ApplyDamage(
						DamagePair.Key, // Character that was hit
						DamagePair.Value, // Apply Body and Head damage combined
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}
			}
		}
		
		
		// FOR SERVER-SIDE REWIND ONLY 
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

		// DEBUG
		if (bDebug)
		{
			// SCATTER
			if(bUseScatter)
			{
				DrawDebugLine(GetWorld(), SphereCenter, EndLoc, FColor::Red, true);
				DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Green, true);
				DrawDebugSphere(GetWorld(), EndLoc, 5.f, 12, FColor::Red, true);
				DrawDebugLine(
					GetWorld(),
					TraceStart,
					FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
					FColor::Cyan,
					true
						);	
			}	
		}
	}
}



