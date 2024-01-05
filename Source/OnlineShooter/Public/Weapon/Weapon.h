#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class AOnlineShooterPlayerController;
class AOnlineShooterCharacter;
class ACasing;
class UWidgetComponent;
class USphereComponent;
class UAnimationAsset;
class UAnimSequence;
class USoundCue;

UENUM()
enum class EWeaponState : uint8
{
	EWS_Initial		UMETA(DisplayName = "Initial State"),
	EWS_Equipped	UMETA(DisplayName = "Equipped"),
	EWS_Dropped		UMETA(DisplayName = "Dropped"),
	EWS_MAX			UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class ONLINESHOOTER_API AWeapon : public AActor 
{
	GENERATED_BODY()
	
public:	

	AWeapon();

	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	
	void ShowPickupWidget(bool bShowWidget);

protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OvelappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
private:

	UPROPERTY()
	AOnlineShooterCharacter* OnlineShooterOwnerCharacter;

	UPROPERTY()
	AOnlineShooterPlayerController* OnlineShooterOwnerController;
	
	// Weapon mesh
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* WeaponMesh;

	// Overlap Area
	UPROPERTY(VisibleAnywhere)
	USphereComponent* AreaSphere;

	// Weapon state
	UPROPERTY(ReplicatedUsing=OnRep_WeaponState, VisibleAnywhere, Category = "Weapon")
	EWeaponState WeaponState;
	
	UPROPERTY(EditAnywhere, Category = "Weapon")
	EWeaponType WeaponType;

	// Pickup Widget
	UPROPERTY(EditAnywhere)
	UWidgetComponent* PickupWidget;

	// Fire animation
	UPROPERTY(EditAnywhere, Category = "Weapon")
	UAnimSequence* FireAnimationSequence;

	// Fire rate (the lower - the faster)
	UPROPERTY(EditAnywhere, Category = "Weapon")
	float FireRate = 1.f;

	// Whether automatic fire or not 
	UPROPERTY(EditAnywhere, Category = "Weapon")
	bool bAutomatic = true;

	// Shells, casings ejected after shot
	UPROPERTY(EditAnywhere)
	TSubclassOf<ACasing> CasingClass;

#pragma region FOV

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ZoomedFOV = 60.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ZoomInterpSpeed = 20.f;

#pragma endregion
	
#pragma region AMMO
	
	UPROPERTY(ReplicatedUsing = OnRep_Ammo, EditAnywhere, Category = "Weapon")
	int32 Ammo;
	
	UPROPERTY(EditAnywhere, Category = "Weapon")
	int32 MagCapacity;

#pragma endregion
	
	// Replication
	UFUNCTION()
	void OnRep_WeaponState();
	
	UFUNCTION()
	void OnRep_Ammo();
	
public:

	/*
	 * Textures for the weapon crosshair
	 */

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairBottom;

	UPROPERTY(EditAnywhere, Category = Weapon)
	USoundCue* EquipSound; 

	UFUNCTION()
	void SetWeaponState(EWeaponState NewState);

	UFUNCTION()
	virtual void Fire(const FVector& HitTarget);

	UFUNCTION()
	void SpendRound();

	UFUNCTION()
	void Dropped();

	UFUNCTION()
	void SetHUDAmmo();

	UFUNCTION()
	bool IsEmpty();

	UFUNCTION()
	void AddAmmo(int32 AmmoToAdd);
	
	FORCEINLINE EWeaponState GetWeaponState() const { return WeaponState; }
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetFireRate() const { return FireRate; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE bool IsAutomatic() const { return  bAutomatic; }
	FORCEINLINE EWeaponType GetWeaponType() const { return  WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
};
