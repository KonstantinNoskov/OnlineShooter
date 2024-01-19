#include "Pickups/Pickup.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Characters/OnlineShooterCharacter.h"
#include "Components/BuffComponent.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/WeaponTypes.h"

APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(RootComponent);
	OverlapSphere->SetSphereRadius(100.f);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	OverlapSphere->AddLocalOffset(FVector(0.f, 0.f, 85.f));

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(OverlapSphere);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupMesh->SetRelativeScale3D(FVector(5.f,5.f,5.f));
	PickupMesh->SetRenderCustomDepth(true);
	PickupMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);

	PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffect"));
	PickupEffectComponent->SetupAttachment(RootComponent);
	PickupEffectComponent->SetRelativeLocation(FVector(0,0, 55.f));
}

void APickup::BeginPlay()
{
	Super::BeginPlay();

	// Delegates server binding
	if(HasAuthority())
	{
		OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnSphereOverlap);	
	}
}

void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PickupMesh)
	{
		PickupMesh->AddWorldRotation(FRotator(0.f, BaseTurnRate * DeltaTime, 0.f));
	}
}

void APickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	OverlapCharacter = !OverlapCharacter ? Cast<AOnlineShooterCharacter>(OtherActor) : OverlapCharacter;
	if(OverlapCharacter)
	{
		Multicast_PlayEffect(OverlapCharacter);	
	}
	
	Destroy();
}

void APickup::Multicast_PlayEffect_Implementation(AOnlineShooterCharacter* OverlappedCharacter)
{
	OverlapCharacter = !OverlapCharacter ? Cast<AOnlineShooterCharacter>(OverlappedCharacter) : OverlapCharacter; 
	
	
	if (PickupEffect && OverlapCharacter && OverlapCharacter->GetMesh())
	{
		UE_LOG(LogTemp, Warning, TEXT("Multicast_PlayEffect"))
		SideEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
		PickupEffect,
		OverlapCharacter->GetMesh(),
		FName(),
		OverlapCharacter->GetActorLocation(),
		OverlapCharacter->GetActorRotation(),
		EAttachLocation::KeepWorldPosition,
		true
		);

		UBuffComponent* BuffComponent = OverlapCharacter->GetBuffComponent();
		if(BuffComponent)
		{
			BuffComponent->SetBuffEffect(SideEffect);
		}
		/*UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		this,
		PickupEffect,
		GetActorLocation(),
		GetActorRotation(),
		FVector(3.f,3.f,3.f)
		);*/
		
	}
}

void APickup::Destroyed()
{
	Super::Destroyed();

	if(PickupSound)
	{
		UGameplayStatics::PlaySound2D(
			this,
			PickupSound
			);
		
		/*UGameplayStatics::PlaySoundAtLocation(
			this,
			PickupSound,
			GetActorLocation()
		);*/
	}
	
	if (PickupEffect && OverlapCharacter && OverlapCharacter->GetMesh())
	{
		/*UNiagaraFunctionLibrary::SpawnSystemAttached(
		PickupEffect,
		OverlapCharacter->GetMesh(),
		FName(),
		OverlapCharacter->GetActorLocation(),
		OverlapCharacter->GetActorRotation(),
		EAttachLocation::KeepWorldPosition,
		true
		);*/

		
	   	/*UNiagaraFunctionLibrary::SpawnSystemAtLocation(
	   	this,
	   	PickupEffect,
	   	GetActorLocation(),
	   	GetActorRotation(),
	   	FVector(3.f,3.f,3.f)
	   	);*/
		
	}
}





