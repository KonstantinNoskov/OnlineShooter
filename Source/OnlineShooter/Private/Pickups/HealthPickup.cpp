#include "Pickups/HealthPickup.h"

#include "Characters/OnlineShooterCharacter.h"
#include "Components/BuffComponent.h"

AHealthPickup::AHealthPickup()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AHealthPickup::BeginPlay()
{
	Super::BeginPlay();
}

void AHealthPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AOnlineShooterCharacter* OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(OtherActor);
	if (OnlineShooterCharacter && OnlineShooterCharacter->GetHealth() < OnlineShooterCharacter->GetMaxHealth())
	{
		UBuffComponent* Buff = Cast<UBuffComponent>(OnlineShooterCharacter->GetBuffComponent());
		if(Buff)
		{
			Buff->Heal(SpawnedPickupValue ? SpawnedPickupValue : HealAmount, HealingTime);
		}

		Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	}
}




