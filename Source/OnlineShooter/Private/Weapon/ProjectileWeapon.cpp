﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileWeapon.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/Projectile.h"


AProjectileWeapon::AProjectileWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AProjectileWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}
void AProjectileWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	if(!HasAuthority()) return; 
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	if(MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());

		// From muzzle flash socket to hit location from TraceUnderCrosshair
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		
		FRotator TargetRotation = ToTarget.Rotation();
		
		if(ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParams;

			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;
			
			UWorld* World = GetWorld();
			if(World)
			{
				World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
			}
		}
	}
}

