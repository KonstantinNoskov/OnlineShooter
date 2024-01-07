#include "Weapon/Shotgun.h"

#include "Characters/OnlineShooterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AShotgun::AShotgun()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AShotgun::BeginPlay()
{
	Super::BeginPlay();
}

void AShotgun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;
	AController* InstigatorController = OwnerPawn->Controller;
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		
		TMap<AOnlineShooterCharacter*, uint32> HitMap;
		
		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			// check if hit actor and Instigator controller is valid; also do server check 
			AOnlineShooterCharacter* OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(FireHit.GetActor());
			if (OnlineShooterCharacter && HasAuthority() && InstigatorController)
			{
				if (HitMap.Contains(OnlineShooterCharacter))
				{
					HitMap[OnlineShooterCharacter]++;
				}

				else
				{
					HitMap.Emplace(OnlineShooterCharacter, 1);
				}
				
				/*UGameplayStatics::ApplyDamage(
				OnlineShooterCharacter,
				Damage,
				InstigatorController,
				this,
				UDamageType::StaticClass()
				);*/
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

		for (auto HitPair : HitMap)
		{
			// check if hit actor and Instigator controller is valid; also do server check 
			if(HitPair.Key && HasAuthority() && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(
				HitPair.Key,
				Damage * HitPair.Value,
				InstigatorController,
				this,
				UDamageType::StaticClass()
				);
			}
		}
	}	
}

void AShotgun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

