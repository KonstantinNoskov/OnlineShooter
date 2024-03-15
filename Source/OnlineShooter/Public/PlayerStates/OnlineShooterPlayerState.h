#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "OnlineShooter/Data/Team.h"
#include "OnlineShooterPlayerState.generated.h"


enum class ETeam : uint8;

class USoundCue;
class AOnlineShooterPlayerController;
class AOnlineShooterCharacter;

UCLASS()
class ONLINESHOOTER_API AOnlineShooterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	
	virtual void OnRep_Score() override;
	virtual  void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	
	UFUNCTION()
	virtual void OnRep_Defeats();

	UFUNCTION()
	void AddToScore(float ScoreAmount);

	UFUNCTION()
	void AddToDefeats(int32 DefeatsAmount);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetAttackerName(const FString& Name);

	UFUNCTION()
	void SetAttackerName(FString Name);

	
private:

	UPROPERTY()
	AOnlineShooterCharacter* Character;

	UPROPERTY()
	AOnlineShooterPlayerController* Controller;

	UPROPERTY(ReplicatedUsing=OnRep_Defeats)
	int32 Defeats = 0;

	UPROPERTY()
	FString KilledBy;


#pragma region TEAMS

	UPROPERTY(Replicated)
	ETeam Team = ETeam::ET_NoTeam;

public:
	
	FORCEINLINE ETeam GetTeam()		const	{ return Team; }
	FORCEINLINE void SetTeam(ETeam NewTeam) { Team = NewTeam; }
	
#pragma endregion

	
#pragma region GAINING THE LEAD

private:
	
	UPROPERTY(EditAnywhere)
	USoundCue* GainLeadAnnounce;

	UPROPERTY(EditAnywhere)
	USoundCue* LostLeadAnnounce;

public:

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayGainLeadAnnounce();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayLostLeadAnnounce();


#pragma endregion

	
};
