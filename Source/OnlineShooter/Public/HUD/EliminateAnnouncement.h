#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EliminateAnnouncement.generated.h"


class UTextBlock;
class UHorizontalBox;

UCLASS()
class ONLINESHOOTER_API UEliminateAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:

	void SetEliminateAnnouncementText(FString AttackerName, FString VictimName);
	
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* AnnouncementBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AnnouncementText;
};
