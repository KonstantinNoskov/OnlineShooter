#include "Pickups/PickupSpawnPoint.h"

#include "Pickups/Pickup.h"
#include "Weapon/Weapon.h"

APickupSpawnPoint::APickupSpawnPoint()
{
	bReplicates = true;

	SpawnPointMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnPointMesh"));
	SpawnPointMesh->SetupAttachment(RootComponent);
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
		SetPickupHighLight();
		
		int32 Selection = FMath::RandRange(0, NumPickupClasses - 1);
		SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClasses[Selection], GetActorTransform());
		SpawnedPickup->SetSpawnPointOwner(this);
		
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

void APickupSpawnPoint::SetPickupHighLight()
{
	bool bPickupHighLightValid =
			SpawnPointMesh
			&& SpawnPointMesh->GetStaticMesh()
			&& SpawnPointMesh->GetStaticMesh()->GetStaticMaterials().Num()
			&& PickupMaterial;
	
	if (bPickupHighLightValid)
	{
		SpawnPointMesh->GetStaticMesh()->SetMaterial(0, PickupMaterial); 
	}
}

void APickupSpawnPoint::SetNoPickupHighLight()
{
	bool bPickupHighLightValid =
			SpawnPointMesh
			&& SpawnPointMesh->GetStaticMesh()
			&& SpawnPointMesh->GetStaticMesh()->GetStaticMaterials().Num()
			&& NoPickupMaterial;
	
	if (bPickupHighLightValid)
	{
		SpawnPointMesh->GetStaticMesh()->SetMaterial(0, NoPickupMaterial); 
	}
}

void APickupSpawnPoint::SpawnPickupFinished()
{
	if (HasAuthority())
	{
		SpawnPickup();
	}
}





