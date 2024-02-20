#include "Components/LagCompensationComponent.h"

#include "OnlineShooter.h"
#include "Characters/OnlineShooterCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/Weapon.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
	
	FFramePackage Package;
	SaveFramePackage(Package);
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	SaveFrameHistory(DeltaTime);
}

void ULagCompensationComponent::SaveFrameHistory(float DeltaTime) 
{
	if (!Character && !Character->HasAuthority()) return;
	
	// In first two frame packages saving to FrameHistory without checking HistoryLength
	if (FrameHistory.Num() <= 1)
	{
		// Save first frame data and put it into head
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}

	// When we have two or more FramePackages, checking for HistoryLength
	else
	{
		// Get HistoryLength in seconds by getting DeltaTime between youngest and oldest frame in history  
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		
		// When history is getting too long deleting old FramePackages until HistoryLength becomes lesser than MaxRecordTime 
		while (HistoryLength > MaxRecordTime)
		{
			// Delete oldest FramePackage
			FrameHistory.RemoveNode(FrameHistory.GetTail());

			// Update HistoryLength 
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;

			if (bDebug)
			{
				DeletedPackagesCount++;
				UE_LOG(LogTemp, Warning, TEXT("ULagCompensationComponent::FillFrameHistory() - Frame Package Removed - %d"), DeletedPackagesCount)
			}
		}

		// Add New FramePackage to the head
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);


		// DEBUG
		if(bDebug)
		{
			ShowFramePackage(ThisFrame, FColor::Yellow, DeltaTime);	
		}
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = !Character ? Cast<AOnlineShooterCharacter>(GetOwner()) : Character;
	if (Character)
	{
		Package.Character = Character;
		Package.Time = GetWorld()->GetTimeSeconds();
		
		for (auto& BoxPair : Character->HitCollisionBoxes)
		{
			FBoxInformation BoxInformation;

			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
			
			Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation); 
		}
	}
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(
	const FFramePackage& OlderFrame,
	const FFramePackage& YoungerFrame,
	float HitTime)
{	
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);
	
	FFramePackage InterpFramePackage = FFramePackage();
	InterpFramePackage.Time = HitTime;

	// Run through all hit boxes and add it's transform info to InterpFramePackage
	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		// Storage for HitBox name
		const FName& BoxInfoName = YoungerPair.Key;

		// Getting hit box transform info of Older and Younger boxes to interpolate between them
		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

		// Storage for InterpFrame HitBox info
		FBoxInformation InterpBoxInfo;

		// Interp location between Younger and Older HitBox location to get InterpFrame HitBox Location 
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpFraction);

		// Interp rotation between Younger and Older HitBox rotation to get InterpFrame HitBox Rotation
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);

		// Keep HitBox size as is
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		// Store all HitBox transform information that we got to InterpFramePackage. 
		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}
	 
	return InterpFramePackage;
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color,float DeltaTime)
{
	if (DrawHitBoxTime > DrawHitBoxFrequency)
	{
		for (auto& BoxInfo : Package.HitBoxInfo)
		{	
			DrawDebugBox(
				GetWorld(),
				BoxInfo.Value.Location,
				BoxInfo.Value.BoxExtent,
				FQuat(BoxInfo.Value.Rotation),
				Color,
				false,
				DrawHitBoxLifeTime
			);
		}

		DrawHitBoxTime = 0.f;
	}
	
	DrawHitBoxTime += DeltaTime;
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(
	const FFramePackage& Package,
	AOnlineShooterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation)
{
	if (HitCharacter) return FServerSideRewindResult();

	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package);
	
	// Disable HitCharacter Mesh collision to be sure our LineTrace wont hit mesh instead HitBox 
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	// Enable collision for the head first
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Block);

	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;

	UWorld* World = GetWorld();
	if (World)
	{
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_Hitbox
			);

		if (ConfirmHitResult.bBlockingHit) // we hit the head, return early
		{
			// Debug
			if (ConfirmHitResult.Component.IsValid()  && bDebug)
			{
				UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				if (Box)
				{
					DrawDebugBox(
						GetWorld(),
						Box->GetComponentLocation(),
						Box->GetScaledBoxExtent(),
						FQuat(Box->GetComponentRotation()),
						FColor::Red,
						false,
						8.f
						);
				}
			}
			
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true, true};
		}
		
		else // didn't hit head, check the rest of the boxes
		{
			for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if (HitBoxPair.Value)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Block);
				}
			}

			World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Hitbox);

			if (ConfirmHitResult.bBlockingHit)
			{
				// Debug
				if (ConfirmHitResult.Component.IsValid() && bDebug)
				{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if (Box)
					{
						DrawDebugBox(
							GetWorld(),
							Box->GetComponentLocation(),
							Box->GetScaledBoxExtent(),
							FQuat(Box->GetComponentRotation()),
							FColor::Blue,
							false,
							8.f
							);
					}
				}
				
				ResetHitBoxes(HitCharacter, CurrentFrame);
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResult{true, false};
			}
		}
	}

	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{false, false};
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(
	const TArray<FFramePackage>& FramePackages,
	const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations)
{

	for(auto& Frame : FramePackages)
	{	
		if (!Frame.Character) return FShotgunServerSideRewindResult();
	}
	
	FShotgunServerSideRewindResult ShotgunResult;
	TArray<FFramePackage> CurrentFrames;
	
	for (auto& Frame : FramePackages)
	{
		FFramePackage CurrentFrame;
		CurrentFrame.Character = Frame.Character;
		CacheBoxPositions(Frame.Character, CurrentFrame);
		
		MoveBoxes(Frame.Character, Frame);

		// Disable HitCharacter Mesh collision to be sure our LineTrace wont hit mesh instead HitBox 
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::NoCollision);

		CurrentFrames.Add(CurrentFrame);
	}

	for(auto& Frame : FramePackages)
	{	
		// Enable collision for the head first
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Block);	
	}

	UWorld* World = GetWorld();

	// Check for headshots
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		
		if (World)
		{
			World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Hitbox);
			
			AOnlineShooterCharacter* OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(ConfirmHitResult.GetActor());
			if (OnlineShooterCharacter)
			{
				// Debug
				if (ConfirmHitResult.Component.IsValid() && bDebug)
				{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if (Box)
					{
						DrawDebugBox(
							GetWorld(),
							Box->GetComponentLocation(),
							Box->GetScaledBoxExtent(),
							FQuat(Box->GetComponentRotation()),
							FColor::Red,
							false,
							8.f
							);
					}
				}

				
				if (ShotgunResult.HeadShots.Contains(OnlineShooterCharacter))
				{
					ShotgunResult.HeadShots[OnlineShooterCharacter]++;
				}
				else
				{
					ShotgunResult.HeadShots.Emplace(OnlineShooterCharacter, 1);
				}
			}
		}	
	}
	
	// enable collision for all boxes, then disable for head box
	for (auto& Frame : FramePackages)
	{
		for (auto& HitBoxPair : Frame.Character->HitCollisionBoxes)
		{
			if (HitBoxPair.Value)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Block);
			}	
		}

		// Enable collision for the head first
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// check for body shots
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		
		if (World)
		{
			World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Hitbox);
			
			AOnlineShooterCharacter* OnlineShooterCharacter = Cast<AOnlineShooterCharacter>(ConfirmHitResult.GetActor());
			if (OnlineShooterCharacter)
			{
				// Debug
				if (ConfirmHitResult.Component.IsValid() && bDebug)
				{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if (Box)
					{
						DrawDebugBox(
							GetWorld(),
							Box->GetComponentLocation(),
							Box->GetScaledBoxExtent(),
							FQuat(Box->GetComponentRotation()),
							FColor::Blue,
							false,
							8.f
							);
					}
				}
				
				if (ShotgunResult.BodyShots.Contains(OnlineShooterCharacter))
				{
					ShotgunResult.BodyShots[OnlineShooterCharacter]++;
				}
				else
				{
					ShotgunResult.BodyShots.Emplace(OnlineShooterCharacter, 1);
				}
			}
		}	
	}
	
	for (auto& Frame : CurrentFrames)
	{
		ResetHitBoxes(Frame.Character, Frame);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::QueryAndPhysics);	
	}
	
	return ShotgunResult;
}

void ULagCompensationComponent::CacheBoxPositions(AOnlineShooterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (!HitCharacter) return;

	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();

			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

// Move hit boxes to the point where there were at the hit moment 
void ULagCompensationComponent::MoveBoxes(AOnlineShooterCharacter* HitCharacter, const FFramePackage& InPackage)
{
	if (!HitCharacter) return;

	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value)
		{
			HitBoxPair.Value->SetWorldLocation(InPackage.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(InPackage.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(InPackage.HitBoxInfo[HitBoxPair.Key].BoxExtent);
		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(AOnlineShooterCharacter* HitCharacter, const FFramePackage& InPackage)
{
	if (!HitCharacter) return;

	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value)
		{
			HitBoxPair.Value->SetWorldLocation(InPackage.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(InPackage.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(InPackage.HitBoxInfo[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(AOnlineShooterCharacter* HitCharacter,
	ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(
	AOnlineShooterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation,
	float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);

	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerSideRewind(
	const TArray<AOnlineShooterCharacter*>& HitCharacters,
	const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations,
	float HitTime)
{
	TArray<FFramePackage> FramesToCheck;

	for (AOnlineShooterCharacter* HitCharacter : HitCharacters)
	{
		FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}
	
	return ShotgunConfirmHit(FramesToCheck, TraceStart, HitLocations);
}


FFramePackage ULagCompensationComponent::GetFrameToCheck(AOnlineShooterCharacter* HitCharacter, float HitTime)
{
	bool bReturn =
		!HitCharacter
	||	!HitCharacter->GetLagCompensation()
	||	!HitCharacter->GetLagCompensation()->FrameHistory.GetHead()
	||	!HitCharacter->GetLagCompensation()->FrameHistory.GetTail();

	if (bReturn) return FFramePackage();

	// Frame Package that we check to verify a hit
	FFramePackage FrameToCheck = FFramePackage();
	bool bShouldInterpolate = true;
	
	// Local storage for the HitCharacter Frame History
	const TDoubleLinkedList<FFramePackage>& HitCharacterHistory = HitCharacter->GetLagCompensation()->FrameHistory;
	const float OldestHistoryTime = HitCharacterHistory.GetTail()->GetValue().Time;
	const float YoungestHistoryTime = HitCharacterHistory.GetHead()->GetValue().Time;
	
	// too far back - too laggy to do Server-Side Rewind
	if (OldestHistoryTime > HitTime) return FFramePackage();

	// if oldest frame time is exactly the same as a HitTime it's our frame to check
	if (OldestHistoryTime == HitTime)
	{
		FrameToCheck = HitCharacterHistory.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	
	if (YoungestHistoryTime <= HitTime)
	{
		FrameToCheck = HitCharacterHistory.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	// At the beginning set oldest and youngest Frames at the same position
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = HitCharacterHistory.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = HitCharacterHistory.GetHead();
	
	while (Older->GetValue().Time > HitTime) // is Older still younger than HitTime?
	{
		// March back until: OlderTime < HitTime < YoungerTime
		if (!Older->GetNextNode()) break;
		Older = Older->GetNextNode();

		if (Older->GetValue().Time > HitTime)
		{
			Younger = Younger->GetNextNode();
		}
	}

	// Highly unlikely case but if we found out our FrameToCheck time is precisely equal to OlderFrame Time. 
	if (Older->GetValue().Time == HitTime)
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}
	
	if (bShouldInterpolate)
	{
		// Interpolate between younger and older
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}

	FrameToCheck.Character = HitCharacter;
	
	return FrameToCheck;
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(
	AOnlineShooterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation,
	float HitTime, AWeapon* DamageCauser)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if (Character && HitCharacter && DamageCauser && Confirm.bHitConfirmed)
	{	
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			DamageCauser->GetDamage(),
			Character->Controller,
			DamageCauser,
			UDamageType::StaticClass());
	}
}

void ULagCompensationComponent::ShotgunServerScoreRequest_Implementation(const TArray<AOnlineShooterCharacter*>& HitCharacters,
	const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime, AWeapon* DamageCauser)
{
	FShotgunServerSideRewindResult Confirm = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);

	for (auto& HitCharacter : HitCharacters)
	{
		if (!HitCharacter || !HitCharacter->GetEquippedWeapon()) continue;

		float TotalDamage = 0.f;
		if (Confirm.HeadShots.Contains(HitCharacter))
		{
			float HeadshotDamage = Confirm.HeadShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetDamage();
			TotalDamage += HeadshotDamage;
		}
		
		if (Confirm.BodyShots.Contains(HitCharacter))
		{
			float BodySHotDamage = Confirm.BodyShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetDamage();
			TotalDamage += BodySHotDamage;
		}
		
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			TotalDamage,
			Character->Controller,
			DamageCauser,
			UDamageType::StaticClass());
	}
}

