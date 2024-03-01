#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "Menu.generated.h"

class UButton;

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebug = false;
	
public:

	UFUNCTION(BlueprintCallable, Category = "Network")
	void MenuSetup(int32 NumberOfPublicConnection = 4, FString TypeOfMatch = FString(TEXT("Noskov")), FString LobbyPath = FString(TEXT("/Game/OnlineShooter/Maps/Lobby/L_Lobby")));

protected:
	
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

protected:
	
	// Callbacks for the custom delegates on the MultiplayerSessionsSubsystem
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

private:
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	UButton* HostButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	UButton* JoinButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	UButton* QuitButton;

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
	FLinearColor ButtonHoverColor{FLinearColor(1,1,1,1)};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
	FLinearColor ButtonDefaultColor{FLinearColor(.3,.3,.3,1)};


private:
	
	UFUNCTION()
	void HostButtonClicked();
	
	UFUNCTION()
	void JoinButtonClicked();

	UFUNCTION()
	void QuitButtonClicked();

	void MenuTearDown();

	UFUNCTION()
	void SetHostButtonColor();

	UFUNCTION()
	void SetJoinButtonColor();

	UFUNCTION()
	void SetQuitButtonColor();
	
	// Reference to a subsystem designed to handle all online session functionality
	UPROPERTY()
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
	
	// Menu setup properties
	int32 NumPublicConnections{4};
	FString MatchType{TEXT("Noskov")};
	FString PathToLobby{TEXT("")};
	
};
