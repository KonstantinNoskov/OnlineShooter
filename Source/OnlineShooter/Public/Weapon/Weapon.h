#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class ACasing;
class UWidgetComponent;
class USphereComponent;
class UAnimationAsset;
class UAnimSequence;

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
	
	void ShowPickupWidget(bool bShowWidget);

protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OvelappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
private:

	// Weapon mesh
	UPROPERTY(VisibleAnywhere, Category = "General Settings")
	USkeletalMeshComponent* WeaponMesh;

	// Overlap Area
	UPROPERTY(VisibleAnywhere, Category = "General Settings")
	USphereComponent* AreaSphere;

	// Weapon state
	UPROPERTY(ReplicatedUsing=OnRep_WeaponState, VisibleAnywhere, Category = "General Settings")
	EWeaponState WeaponState;

	// Pickup Widget
	UPROPERTY(EditAnywhere, Category = "General Settings")
	UWidgetComponent* PickupWidget;

	// Fire animation
	UPROPERTY(EditAnywhere, Category = "General Settings")
	UAnimSequence* FireAnimationSequence;

	// Fire rate (the lower - the faster)
	UPROPERTY(EditAnywhere, Category = "General Settings")
	float FireRate = 1.f;

	// Whether automatic fire or not 
	UPROPERTY(EditAnywhere, Category = "General Settings")
	bool bAutomatic = true;

	// Shells, casings ejected after shot
	UPROPERTY(EditAnywhere)
	TSubclassOf<ACasing> CasingClass;

	/*
	 *  Zoomed FOV while aiming
	 */

	UPROPERTY(EditDefaultsOnly, Category = "General Settings")
	float ZoomedFOV = 60.f;

	UPROPERTY(EditDefaultsOnly, Category = "General Settings")
	float ZoomInterpSpeed = 20.f;
	
	// Replication
	UFUNCTION()
	void OnRep_WeaponState();
	
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
	
	void SetWeaponState(EWeaponState NewState);
	virtual void Fire(const FVector& HitTarget);

	UFUNCTION()
	void Dropped();
	
	
	FORCEINLINE EWeaponState GetWeaponState() const { return WeaponState; }
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetFireRate() const { return FireRate; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE bool IsAutomatic() const { return  bAutomatic; }
	
	
};
