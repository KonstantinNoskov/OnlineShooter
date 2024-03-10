#include "Weapon/HitScanWeapon.h"

#include "Characters/OnlineShooterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/LagCompensationComponent.h"
#include "PlayerController/OnlineShooterPlayerController.h"
#include "Sound/SoundCue.h"


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
		if(OnlineShooterCharacter && InstigatorController)
		{
			bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
			if (HasAuthority() && bCauseAuthDamage)
			{
				float DamageToCause = FireHit.BoneName.ToString() == FString("head") ? Damage * CritFactor : Damage;
				
				UGameplayStatics::ApplyDamage(
				OnlineShooterCharacter,
				DamageToCause,
				InstigatorController,
				this,
				UDamageType::StaticClass()
				);
			}

			if (!HasAuthority() && bUseServerSideRewind)
			{
				OnlineShooterOwnerCharacter = !OnlineShooterOwnerCharacter ? Cast<AOnlineShooterCharacter>(OwnerPawn) : OnlineShooterOwnerCharacter;
				OnlineShooterOwnerController = !OnlineShooterOwnerController ? Cast<AOnlineShooterPlayerController>(InstigatorController) : OnlineShooterOwnerController;
				
				if (OnlineShooterOwnerCharacter && OnlineShooterOwnerController && OnlineShooterOwnerCharacter->GetLagCompensation() && OnlineShooterOwnerCharacter->IsLocallyControlled())
				{
					OnlineShooterOwnerCharacter->GetLagCompensation()->ServerScoreRequest(
						OnlineShooterCharacter,
						Start,
						HitTarget,
						OnlineShooterOwnerController->GetServerTime() - OnlineShooterOwnerController->GetSingleTripTime(),
						this
					);
				}
			}
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
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f  ;
		
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
		}
		else
		{
			OutHit.ImpactPoint = End;
		}

		// DEBUG
		if(bDebug) DrawDebugSphere(GetWorld(), BeamEnd, 16.f, 12, FColor::Orange, true);
		
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


