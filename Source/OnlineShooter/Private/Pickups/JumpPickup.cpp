#include "Pickups/JumpPickup.h"

#include "Characters/OnlineShooterCharacter.h"
#include "Components/BuffComponent.h"


AJumpPickup::AJumpPickup()
{
	PrimaryActorTick.bCanEverTick = true; 
}

void AJumpPickup::BeginPlay()
{
	Super::BeginPlay();
}

void AJumpPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AJumpPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AOnlineShooterCharacter* OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(OtherActor);
	if (OnlineShooterCharacter)
	{
		UBuffComponent* Buff = Cast<UBuffComponent>(OnlineShooterCharacter->GetBuffComponent());
		if(Buff)
		{
			Buff->BuffJump( SpawnedPickupValue ? SpawnedPickupValue : JumpZVelocityBuff, JumpBuffTime);
		}
	}
}