#include "Pickups/PickupSpawnPoint.h"

#include "Kismet/GameplayStatics.h"
#include "MaterialEditor/Public/MaterialEditingLibrary.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Pickups/Pickup.h"

APickupSpawnPoint::APickupSpawnPoint()
{
	bReplicates = true;

	SpawnPointMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnPointMesh"));
	SpawnPointMesh->SetupAttachment(RootComponent);
}

void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SpawnPickup();	
	}
	
	
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
		SetSpawnPointHighLight(false);

		if (SpawnSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this,SpawnSound, this->GetActorLocation());	
		}
		
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
	UE_LOG(LogTemp, Error, TEXT("APickupSpawnPoint::StartSpawnPickupTimer()"))
	const float SpawnTime = FMath::FRandRange(SpawnPickupTimeMin, SpawnPickupTimeMax);
	GetWorldTimerManager().SetTimer(SpawnPickupTimer, this, &ThisClass::SpawnPickupFinished, SpawnTime);
}

void APickupSpawnPoint::SetSpawnPointHighLight(bool bPickedUp)
{
	bool bPickupHighLightValid =
			SpawnPointMesh
			&& SpawnPointMesh->GetStaticMesh()
			&& SpawnPointMesh->GetStaticMesh()->GetStaticMaterials().Num();

	
	if (bPickupHighLightValid)
	{	
		UMaterialInstance* SpawnPadMaterial = bPickedUp ? SpawnOffMaterial : SpawnOnMaterial;
		if (SpawnPadMaterial) SpawnPointMesh->SetMaterial(0, SpawnPadMaterial);
	}
}

void APickupSpawnPoint::SpawnPickupFinished()
{
	UE_LOG(LogTemp, Error, TEXT("APickupSpawnPoint::SpawnPickupFinished()"))
	if (HasAuthority())
	{
		SpawnPickup();
		UE_LOG(LogTemp, Error, TEXT("APickupSpawnPoint::SpawnPickup()"))
	}
}





