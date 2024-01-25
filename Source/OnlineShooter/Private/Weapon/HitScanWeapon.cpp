#include "Weapon/HitScanWeapon.h"

#include "Characters/OnlineShooterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"
#include "WeaponTypes.generated.h"


AHitScanWeapon::AHitScanWeapon()
{
}

void AHitScanWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AHitScanWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(!OwnerPawn) return;

	AController* InstigatorController = OwnerPawn->Controller;
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if(MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		// check if hit actor and Instigator controller is valid; also do server check 
		AOnlineShooterCharacter* OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(FireHit.GetActor());
		if(OnlineShooterCharacter && HasAuthority() && InstigatorController)
		{
			UGameplayStatics::ApplyDamage(
			OnlineShooterCharacter,
			Damage,
			InstigatorController,
			this,
			UDamageType::StaticClass()
			);
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
				FireHit.ImpactPoint
			);
		}
		
		//FVector End = Start + (HitTarget - Start) * 1.25f;
		
		/*UWorld* World = GetWorld();
		if(World)
		{
			World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECC_Visibility
			);

			FVector BeamEnd = End;

			// Bullet trace hit check
			if(FireHit.bBlockingHit) 
			{
				// Set beam end point to hit impact point
				BeamEnd = FireHit.ImpactPoint;

				// check if hit actor and Instigator controller is valid; also do server check 
				AOnlineShooterCharacter* OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(FireHit.GetActor());
				if(OnlineShooterCharacter && HasAuthority() && InstigatorController)
				{
					UGameplayStatics::ApplyDamage(
					OnlineShooterCharacter,
					Damage,
					InstigatorController,
					this,
					UDamageType::StaticClass()
					);
				}

				// Play impact particles at the impact point
				if(ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						World,
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
						FireHit.ImpactPoint
					);
				}
			}

			// Beam particles valid check
			if(BeamParticles)
			{
				// Perform smoke trail particles along the bullet trace
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation( 
					World,
					BeamParticles,
					SocketTransform
				);

				// ?? Set vector param ?? 
				if(Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}*/
		
		// Perform muzzle flash particle if valid 
		if(MuzzleFlash)
		{	
			UGameplayStatics::SpawnEmitterAtLocation( 
				GetWorld(),
				MuzzleFlash,
				SocketTransform
			);
		}

		// Play fire sound if valid
		if(FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
	}
} 

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit) 
{
	UWorld* World = GetWorld();
	if(World)
	{
		FVector End = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f  ;
		
		World->LineTraceSingleByChannel(
				OutHit,
				TraceStart,
				End,
				ECC_Visibility
			);

		FVector BeamEnd = End;

		if(OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;

			// Beam particles valid check
			if(BeamParticles)
			{
				// Perform smoke trail particles along the bullet trace
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation( 
					World,
					BeamParticles,
					TraceStart,
					FRotator::ZeroRotator,
					true
				);

				// ?? Set vector param ?? 
				if(Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}
	}
}

// Defines pellets scatter 
FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	FVector EndLoc = SphereCenter + RandVec;
	FVector ToEndLoc = EndLoc - TraceStart;
	
	
	/*DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	DrawDebugLine(
		GetWorld(),
		TraceStart,
		FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
			FColor::Cyan,
			true
			);*/
	
	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}


