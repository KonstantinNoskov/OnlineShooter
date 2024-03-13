
#include "HUD/OnlineShooterHUD.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "HUD/Announcement.h"
#include "HUD/CharacterOverlay.h"
#include "HUD/Chat.h"
#include "HUD/SniperScopeWidget.h"
#include "PlayerController/OnlineShooterPlayerController.h"
#include "PlayerStates/OnlineShooterPlayerState.h"

void AOnlineShooterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if(GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f , ViewportSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;
		
		if (HUDPackage.CrosshairCenter)
		{
			FVector2d Spread(0.f,0.f);
			DrawCrosshair(HUDPackage.CrosshairCenter, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}

		if (HUDPackage.CrosshairLeft)
		{
			FVector2d Spread(-SpreadScaled,0.f);
			DrawCrosshair(HUDPackage.CrosshairLeft, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}

		if (HUDPackage.CrosshairRight)
		{
			FVector2d Spread(SpreadScaled,0.f);
			DrawCrosshair(HUDPackage.CrosshairRight, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}

		if (HUDPackage.CrosshairTop)
		{
			FVector2d Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairTop, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}

		if (HUDPackage.CrosshairBottom)
		{
			FVector2d Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairBottom, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
	}
}

void AOnlineShooterHUD::BeginPlay()
{
	Super::BeginPlay();
}

void AOnlineShooterHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();

	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void AOnlineShooterHUD::AddAnnouncement()
{
	OwningPlayer = !OwningPlayer ? GetOwningPlayerController() : OwningPlayer;

	if (OwningPlayer && AnnouncementClass)
	{
		Announcement = CreateWidget<UAnnouncement>(OwningPlayer, AnnouncementClass);
		Announcement->AddToViewport();
	}
}
void AOnlineShooterHUD::AddSniperScope()
{
	OwningPlayer = !OwningPlayer ? GetOwningPlayerController() : OwningPlayer;

	if (OwningPlayer && SniperScopeClass)
	{
		SniperScope = CreateWidget<USniperScopeWidget>(OwningPlayer, SniperScopeClass);
		
		if(SniperScope && SniperScope->ScopeOverlay && SniperScope->Background)
		{
			SniperScope->AddToViewport();
		}
		
	}
}

#pragma region CHAT 

UChat* AOnlineShooterHUD::AddChat()
{
	OwningPlayer = !OwningPlayer ? GetOwningPlayerController() : OwningPlayer;
	if (OwningPlayer && ChatClass)
	{
		if (!ChatWidget)
		{
			ChatWidget = CreateWidget<UChat>(OwningPlayer, ChatClass);

			if (ChatWidget)
			{
				ChatWidget->ShowChat();
				ChatWidget->GetChatInput()->OnTextCommitted.AddDynamic(this, &AOnlineShooterHUD::OnInputCommitted);
				return ChatWidget;
			}
		}
	}

	return nullptr;
}
void AOnlineShooterHUD::RemoveChat()
{
	if (ChatWidget)
	{
		ChatWidget->ChatTearDown();
		ChatWidget->GetChatInput()->OnTextCommitted.RemoveDynamic(this, &AOnlineShooterHUD::OnInputCommitted);
		ChatWidget = nullptr;
	}
}

void AOnlineShooterHUD::OnInputCommitted(const FText& InText, ETextCommit::Type CommitMethod)
{
	if(CommitMethod != ETextCommit::OnEnter) return;
	
	AOnlineShooterPlayerController* PlayerController = Cast<AOnlineShooterPlayerController>(GetOwningPlayerController());
	if (ChatWidget && PlayerController && PlayerController->GetPlayerState<AOnlineShooterPlayerState>())
	{
		/*for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			AOnlineShooterPlayerController* Player = Cast<AOnlineShooterPlayerController>(*It);
			if (Player)
			{
				
			}
		}*/

		PlayerController->BroadcastChatMessage(PlayerController->GetPlayerState<AOnlineShooterPlayerState>(), InText.ToString());	
	}
}

#pragma endregion

void AOnlineShooterHUD::AddEliminateAnnouncement(FString Attacker, FString Victim)
{
	OwningPlayer = !OwningPlayer ? GetOwningPlayerController() : OwningPlayer;
	if (OwningPlayer && EliminateAnnouncementClass)
	{
		EliminateAnnouncementWidget = CreateWidget<UEliminateAnnouncement>(OwningPlayer, EliminateAnnouncementClass);
		if (EliminateAnnouncementWidget)
		{
			EliminateAnnouncementWidget->SetEliminateAnnouncementText(Attacker, Victim);
			EliminateAnnouncementWidget->AddToViewport();

			for (UEliminateAnnouncement* Message : EliminateMessages)
			{
				if (Message && Message->AnnouncementBox)
				{
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Message->AnnouncementBox);
					if (CanvasSlot)
					{
						FVector2D Position = CanvasSlot->GetPosition();
						FVector2D NewPosition(CanvasSlot->GetPosition().X, Position.Y + CanvasSlot->GetSize().Y);

						CanvasSlot->SetPosition(NewPosition);
					}
				}
			}
			
			EliminateMessages.Add(EliminateAnnouncementWidget);
			
			FTimerHandle EliminateMessageTimer;
			FTimerDelegate EliminateMessageDelegate; 

			EliminateMessageDelegate.BindUFunction(this, FName("EliminateAnnounceTimerFinished"), EliminateAnnouncementWidget);
			GetWorldTimerManager().SetTimer(
				EliminateMessageTimer,
				EliminateMessageDelegate,
				EliminateAnnouncementTime,
				false
				);
		}
	}
}

void AOnlineShooterHUD::EliminateAnnounceTimerFinished(UEliminateAnnouncement* MessageToRemove)
{
	if (MessageToRemove)
	{
		MessageToRemove->RemoveFromParent();
	}
}

void AOnlineShooterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2d Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHight / 2.f) + Spread.Y
	);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrosshairColor
	);
}
