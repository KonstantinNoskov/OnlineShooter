#include "Pickups/PickupSpawnPoint.h"

#include "Pickups/Pickup.h"

APickupSpawnPoint::APickupSpawnPoint()
{
	bReplicates = true;
}

void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	SpawnPickup();
	
	//StartSpawnPickupTimer((AActor*)nullptr);
}

void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APickupSpawnPoint::SpawnPickup()
{
	int32 NumPickupClasses = PickupClasses.Num();

	if (NumPickupClasses)
	{
		int32 Selection = FMath::RandRange(0, NumPickupClasses - 1);
		SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClasses[Selection], GetActorTransform());

		if (HasAuthority() && SpawnedPickup)
		{
			if(bSpawnLoop)
			{
				SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnPickupTimer);		
			}

			else if(SpawnRepeatAmount)
			{
				SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnPickupTimer);
				SpawnRepeatAmount--;
			}
		}
	}
}

void APickupSpawnPoint::StartSpawnPickupTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::FRandRange(SpawnPickupTimeMin, SpawnPickupTimeMax);
	GetWorldTimerManager().SetTimer(SpawnPickupTimer, this, &APickupSpawnPoint::SpawnPickupFinished, SpawnTime);
}

void APickupSpawnPoint::SpawnPickupFinished()
{
	if (HasAuthority())
	{
		SpawnPickup();
	}
}





