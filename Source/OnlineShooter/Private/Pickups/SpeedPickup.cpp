#include "Pickups/SpeedPickup.h"

#include "NiagaraComponent.h"
#include "Characters/OnlineShooterCharacter.h"
#include "Components/BuffComponent.h"


ASpeedPickup::ASpeedPickup()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASpeedPickup::BeginPlay()
{
	Super::BeginPlay();
}

void ASpeedPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ASpeedPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AOnlineShooterCharacter* OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(OtherActor);
	if (OnlineShooterCharacter)
	{
		UBuffComponent* Buff = Cast<UBuffComponent>(OnlineShooterCharacter->GetBuffComponent());
		if(Buff)
		{
			Buff->BuffSpeed(BaseSpeedBuff, CrouchSpeedBuff, SpeedBuffTime);
		}
	}

	Destroy();
}


