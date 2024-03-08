#include "HUD/EliminateAnnouncement.h"

#include "Components/TextBlock.h"

void UEliminateAnnouncement::SetEliminateAnnouncementText(FString AttackerName, FString VictimName)
{
	FString EliminateAnnouncementText = FString::Printf(TEXT("%s killed %s"), *AttackerName, *VictimName);
	if (AnnouncementText)
	{
		AnnouncementText->SetText(FText::FromString(EliminateAnnouncementText));
	}
}
