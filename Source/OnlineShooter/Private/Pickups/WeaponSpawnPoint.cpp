#include "Pickups/WeaponSpawnPoint.h"

#include "Kismet/GameplayStatics.h"
#include "Weapon/Weapon.h"


// Sets default values
AWeaponSpawnPoint::AWeaponSpawnPoint()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AWeaponSpawnPoint::SpawnPickup()
{
	int32 NumWeaponClasses = WeaponClasses.Num();
	
	if (NumWeaponClasses)
	{
		SetSpawnPointHighLight(false);

		if (SpawnSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this,SpawnSound, this->GetActorLocation());	
		}
		
		int32 Selection = FMath::RandRange(0, NumWeaponClasses - 1);
		
		SpawnedWeapon = GetWorld()->SpawnActor<AWeapon>(
			WeaponClasses[Selection],
			GetActorLocation() + FVector(0.f, 15.f, 100.f),
			GetActorRotation());

		SpawnedWeapon->GetWeaponMesh()->SetRelativeScale3D(FVector(2.f, 2.f, 2.f));
		SpawnedWeapon->SetSpawnPointOwner(this);
		SpawnedWeapon->bDestroyWeapon = true;
		
		if (HasAuthority() && SpawnedWeapon)
		{
			if(bSpawnLoop)
			{
				//SpawnedWeapon->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnPickupTimer);
			}

			else if(SpawnRepeatAmount)
			{
				SpawnedWeapon->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnPickupTimer);
				SpawnRepeatAmount--;
			}
		}
	}
}

void AWeaponSpawnPoint::SetSpawnPointHighLight(bool bPickedUp)
{
	Super::SetSpawnPointHighLight(bPickedUp);
}


