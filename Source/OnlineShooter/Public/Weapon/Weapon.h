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

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial				UMETA(DisplayName = "Initial State"),
	EWS_Equipped			UMETA(DisplayName = "Equipped"),
	EWS_Dropped				UMETA(DisplayName = "Dropped"),
	EWS_EquippedSecondary	UMETA(DisplayName = "EquippedSecondary"),
	EWS_MAX					UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan		UMETA(DisplayName = "HitScan Weapon"),
	EFT_Projectile	UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun		UMETA(DisplayName = "Shotgun Weapon"),
	EFT_MAX			UMETA(DisplayName = "DefaultMAX")
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
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	virtual void OnWeaponStateSet();
	virtual void HandleWeaponEquipped();
	virtual void HandleWeaponDropped();
	virtual void HandleEquippedSecondary();
	
private:
	
	// Weapon mesh
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditAnywhere)
	float WeaponMeshTurnRate = 45.f;

	// Overlap Area
	UPROPERTY(VisibleAnywhere)
	USphereComponent* AreaSphere;

	// Weapon state
	UPROPERTY(ReplicatedUsing=OnRep_WeaponState, VisibleAnywhere, Category = "Weapon")
	EWeaponState WeaponState = EWeaponState::EWS_Initial;
	
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

protected:

	UPROPERTY()
	AOnlineShooterCharacter* OnlineShooterOwnerCharacter;

	UPROPERTY()
	AOnlineShooterPlayerController* OnlineShooterOwnerController;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter", meta = (EditCondition = "bUseScatter"))
	float DistanceToSphere = 800.f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter", meta = (EditCondition = "bUseScatter"))
	float SphereRadius = 75.f;
	
	UPROPERTY(EditAnywhere)
	bool bDebug = false;

	UPROPERTY(EditAnywhere)
	float Damage = 5.f;

	UPROPERTY(EditAnywhere)
	bool bUseServerSideRewind = false;

public:

	UPROPERTY()
	bool bDestroyWeapon = false;
	
	UPROPERTY(EditAnywhere)
	USoundCue* ZoomInSound;

	UPROPERTY(EditAnywhere)
	USoundCue* ZoomOutSound;

#pragma region FOV

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ZoomedFOV = 60.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ZoomInterpSpeed = 20.f;

#pragma endregion
	
#pragma region AMMO
	
	UPROPERTY(EditAnywhere, Category = "Weapon")
	int32 Ammo;
	
	UPROPERTY(EditAnywhere, Category = "Weapon")
	int32 MagCapacity;

	// The nu,bet of unprocessed server requests for ammo.
	// Incremented in SpendRound(), decremented in ClientUpdateAmmo()
	UPROPERTY()
	int32 Sequence = 0;

	UFUNCTION(Client, Reliable)
	void Client_UpdateAmmo(int32 ServerAmmo);
	
	UFUNCTION(Client, Reliable)
	void Client_AddAmmo(int32 AmmoToAdd);
	
#pragma endregion
	
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

	UPROPERTY(EditAnywhere)
	EFireType FireType;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;

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
	bool IsMagEmpty();

	UFUNCTION()
	bool IsMagFull();

	UFUNCTION()
	void AddAmmo(int32 AmmoToAdd);

	UFUNCTION()
	void EnableCustomDepth(bool bEnable);

	UFUNCTION()
	FVector TraceEndWithScatter( const FVector& HitTarget);

	// Replication
	UFUNCTION()
	void OnRep_WeaponState();
	
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
	FORCEINLINE float GetDamage() const { return Damage; }
};
