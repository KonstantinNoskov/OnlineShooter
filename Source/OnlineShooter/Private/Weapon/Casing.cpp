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


	// Ejection impulse in floats
	EjectionRightImpulseMin = 1.f;	// Right min
	EjectionRightImpulseMax = 5.f;	// Right max

	EjectionUpImpulseMin = 1.f;		// Up min
	EjectionUpImpulseMax = 5.f;		// Up max
	
	EjectionFwdImpulseMin = 1.f;	// Fwd min
	EjectionFwdImpulseMax = 3.f;	// Fwd max

	// Rotation Impulse for ejected casings in degrees
	CasingSpinYawMin = 360.f;		// Yaw min
	CasingSpinYawMax = 1440.f;		// Yaw max
	
	CasingSpinPitchMin = 0.f;		// Pitch min
	CasingSpinPitchMax = 0.f;		// Pitch max
	
	CasingSpinRollMin = 0.f;		// Roll min
	CasingSpinRollMax = 0.f;		// Roll max

	// LIFETIME
	CasingLifeTime = 3.f;
	
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();

	// Ejected casing random rotation 
	float RandYaw = FMath::FRandRange(CasingSpinYawMin, CasingSpinYawMax);
	float RandPitch = FMath::FRandRange(CasingSpinPitchMin, CasingSpinPitchMax);
	float RandRoll = FMath::FRandRange(CasingSpinRollMin, CasingSpinRollMax);
	FVector CasingRotationImpulse = FVector(0, 0, RandYaw);
	CasingMesh->AddAngularImpulseInDegrees(CasingRotationImpulse);

	// Ejected casing impulse
	float RandRightImpulse = FMath::FRandRange(EjectionRightImpulseMin, EjectionRightImpulseMax);
	float RandUpImpulse = FMath::FRandRange(EjectionUpImpulseMin, EjectionUpImpulseMax);
	float RandFwdImpulse = FMath::FRandRange(EjectionFwdImpulseMin, EjectionFwdImpulseMax);
	FVector CasingEjectImpulse(RandRightImpulse, RandUpImpulse, RandFwdImpulse);
	
	CasingMesh->AddImpulse((GetActorRightVector() * RandRightImpulse + GetActorForwardVector() * RandUpImpulse * -1 + GetActorUpVector() * 2.f));
	
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

