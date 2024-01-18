#include "Pickups/HealthPickup.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Characters/OnlineShooterCharacter.h"
#include "Components/BuffComponent.h"

AHealthPickup::AHealthPickup()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffect"));
	PickupEffectComponent->SetupAttachment(RootComponent);
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
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AOnlineShooterCharacter* OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(OtherActor);
	if (OnlineShooterCharacter)
	{
		UBuffComponent* Buff = Cast<UBuffComponent>(OnlineShooterCharacter->GetBuffComponent());
		if(Buff)
		{
			Buff->Heal(HealAmount, HealingTime);
		}
	}

	Destroy();
}

void AHealthPickup::Destroyed()
{
	if (PickupEffect) // Pickup effect valid check
	{
		 UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			PickupEffect,
			GetActorLocation(),
			GetActorRotation(),
			FVector(3.f,3.f,3.f)
		);

		
	}

	Super::Destroyed();
}


