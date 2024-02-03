#include "Components/LagCompensationComponent.h"

#include "Characters/OnlineShooterCharacter.h"
#include "Components/BoxComponent.h"

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

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = !Character ? Cast<AOnlineShooterCharacter>(GetOwner()) : Character;
	if (Character)
	{
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

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame,
	const FFramePackage& YoungerFrame, float HitTime)
{	
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);
	
	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;

	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;

		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

		FBoxInformation InterpBoxInfo;

		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}
	 
	return FFramePackage();
}

void ULagCompensationComponent::SaveFrameHistory(float DeltaTime)
{
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
		
		// When history is getting too long deleting old FramePackages to the point where HistoryLength becomes lesser than MaxRecordTime 
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

		if(bDebug)
		{
			ShowFramePackage(ThisFrame, FColor::Yellow, DeltaTime);	
		}
	}
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

void ULagCompensationComponent::ServerSideRewind(AOnlineShooterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	bool bReturn =
		!HitCharacter
	|| HitCharacter->GetLagCompensation()
	|| !HitCharacter->GetLagCompensation()->FrameHistory.GetHead()
	|| !HitCharacter->GetLagCompensation()->FrameHistory.GetTail();

	// Frame Package that we check to verify a hit
	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;
	
	// Local storage for the HitCharacter Frame History
	const TDoubleLinkedList<FFramePackage>& HitCharacterHistory = HitCharacter->GetLagCompensation()->FrameHistory;
	const float OldestHistoryTime = HitCharacterHistory.GetTail()->GetValue().Time;
	const float YoungestHistoryTime = HitCharacterHistory.GetHead()->GetValue().Time;
	
	// too far back - too laggy to handle Server-Side Rewind
	if (OldestHistoryTime > HitTime) return;

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
	
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = HitCharacterHistory.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = HitCharacterHistory.GetHead();
	
	while (Older->GetValue().Time > HitTime) // is Older still younger than HitTime?
	{
		// March back until: OlderTime < HitTime < YoungerTime
		if (!Older->GetNextNode()) return;
		Older = Older->GetNextNode();

		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}

	// Highly unlikely case but if we found our FrameToCheck time is precisely equal to OlderFrame Time. 
	if (Older->GetValue().Time == HitTime)
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}
	
	if (bShouldInterpolate)
	{
		// Interpolate between younger and older
	}
	
	if (bReturn) return;
}

