#include "Pickups/ShieldPickup.h"

#include "Characters/OnlineShooterCharacter.h"
#include "Components/BuffComponent.h"


AShieldPickup::AShieldPickup()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AShieldPickup::BeginPlay()
{
	Super::BeginPlay();

	
}

void AShieldPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AShieldPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AOnlineShooterCharacter* OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(OtherActor);
	if (OnlineShooterCharacter)
	{
		UBuffComponent* Buff = Cast<UBuffComponent>(OnlineShooterCharacter->GetBuffComponent());
		if(Buff && OnlineShooterCharacter->GetShield() < OnlineShooterCharacter->GetMaxShield())
		{
			UE_LOG(LogTemp, Warning, TEXT("%f"), Buff->GetShieldReplenishAmount())
			Buff->ReplenishShield(SpawnedPickupValue ? SpawnedPickupValue : ShieldReplenishAmount, ShieldReplenishTime);
			Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
		}
	}
}