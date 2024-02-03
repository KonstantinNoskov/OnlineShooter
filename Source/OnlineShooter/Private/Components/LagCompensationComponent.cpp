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

