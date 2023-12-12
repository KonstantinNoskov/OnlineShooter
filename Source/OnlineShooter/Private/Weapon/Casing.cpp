#include "Weapon/Casing.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = true;
	
	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletShellMesh"));
	SetRootComponent(CasingMesh);

	CasingMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CasingMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);
	ShellEjectionImpulse = 10.f;

	// Rotation Impulse for ejected casings in degrees
	CasingSpinYaw = 360.f;
	CasingSpinPitch = 0.f;
	CasingSpinRoll = 0.f;
	
	CasingLifeTime = 3.f;
	
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();
	
	CasingMesh->AddImpulse(GetActorRightVector() * ShellEjectionImpulse + GetActorForwardVector() * (-2.f) );
	CasingMesh->AddAngularImpulseInDegrees(FVector(CasingSpinRoll,CasingSpinPitch,CasingSpinYaw), NAME_None, true);

	
	CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);
	SetLifeSpan(CasingLifeTime);
	
}

void ACasing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if(ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());	
	}
	CasingMesh->SetNotifyRigidBodyCollision(false);

	
	//Destroy();
}

