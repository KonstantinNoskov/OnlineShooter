#include "Weapon/HitScanWeapon.h"

#include "Characters/OnlineShooterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"


AHitScanWeapon::AHitScanWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	
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
		FVector End = Start + (HitTarget - Start) * 1.25f;

		FHitResult FireHit;

		UWorld* World = GetWorld();
		if(World)
		{
			World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECC_Visibility
			);

			FVector BeamEnd = End;

			if(FireHit.bBlockingHit)
			{
				BeamEnd = FireHit.ImpactPoint;
				
				AOnlineShooterCharacter* OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(FireHit.GetActor());
				if(OnlineShooterCharacter && HasAuthority() && InstigatorController)
				{
					if(HasAuthority())
					{
						UGameplayStatics::ApplyDamage(
						OnlineShooterCharacter,
						Damage,
						InstigatorController,
						this,
						UDamageType::StaticClass()
						);
					}
				}

				if(ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						World,
						ImpactParticles,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()
					);
				}

				if (ImpactSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
						this,
						ImpactSound,
						FireHit.ImpactPoint
					);
				}
				
			}

			if(BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					World,
					BeamParticles,
					SocketTransform
				);

				if(Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}

		if(MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				World,
				MuzzleFlash,
				SocketTransform
			);
		}

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

