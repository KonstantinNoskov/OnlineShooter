#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OnlineShooter/Data/TurningInPlace.h"

// INPUT
#include "InputActionValue.h"
#include "Components/CombatComponent.h"

// INTERFACES
#include "Interfaces/InteractInterface.h"

// TIMELINE
#include "Components/TimelineComponent.h"

#include "OnlineShooterCharacter.generated.h"

class AOnlineShooterGameMode;
enum class ETeam : uint8;
class UNiagaraComponent;
class UNiagaraSystem;
class ULagCompensationComponent;
class UBoxComponent;
class UBuffComponent;
class UWidgetComponent;
class USpringArmComponent;
class UCameraComponent;
class UCombatComponent;
class AOnlineShooterPlayerState;
class USoundCue;
class AWeapon;

// INPUT
class UInputMappingContext;
class UInputAction;
struct FInputActionInstance;

// DELEGATES
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

UCLASS()
class ONLINESHOOTER_API AOnlineShooterCharacter : public ACharacter, public IInteractInterface
{
	GENERATED_BODY()

public:

	// Constructor
	AOnlineShooterCharacter();

	// Basic overrides
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void OnRep_ReplicatedMovement() override;
	virtual void Destroyed() override;

	// Friend Classes
	friend UCombatComponent;

protected:
	
	virtual void BeginPlay() override;
	virtual void Jump() override;

private:

#pragma region COMPONENTS
	
	// Camera boom positioning the camera behind the character 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	// Follow camera 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	// OverHead Widget 
	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PlayerWidget, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;*/

	// Combat component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* Combat;

	// Buff component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Buff, meta = (AllowPrivateAccess = "true"))
	UBuffComponent* Buff;
	
	// Grenade
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* AttachedGrenade;

	// Lag Compensation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	ULagCompensationComponent* LagCompensation;

#pragma endregion

#pragma region INPUT
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* CharacterMappingContext;
	
	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Crouch Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;
	
	/** Equip Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipAction;

	/** Aim Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	/** Reload Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;

	/** Throw grenade Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ThrowGrenadeAction;

	UPROPERTY(EditAnywhere)
	USoundCue* JumpStartSound;

	UPROPERTY(EditAnywhere)
	float AimSensitivity = .1f;
	
protected:

	
	
	/** Callback for movement input */
	UFUNCTION()
	void Move(const FInputActionValue& Value);

	/** Callback for looking input */
	UFUNCTION()
	void Look(const FInputActionValue& Value);

	/** Callback for Equip input */
	UFUNCTION()
	void EquipButtonPressed();

	/** Callback for Crouch input */
	UFUNCTION()
	void CrouchButtonPressed();
	
	/** Callback for Crouch input */
	UFUNCTION()
	void CrouchButtonReleased();

	/** Callback for Aim input */
	UFUNCTION()
	void AimButtonPressed();

	UFUNCTION()
	void AimButtonReleased();

	UFUNCTION()
	void CalculateAO_Pitch();
	
	UFUNCTION()
	void FireButtonPressed();
	
	UFUNCTION()
	void FireButtonReleased();

	UFUNCTION()
	void ReloadButtonPressed();

	UFUNCTION()
	void GrenadeButtonPressed();

private:

	// Servers Inputs
	UFUNCTION(Server, Reliable)
	void Server_EquipButtonPressed();

#pragma endregion

private:

	UPROPERTY(EditAnywhere)
	bool bDebug = false;
	
	UPROPERTY()
	AOnlineShooterPlayerController* OnlineShooterPlayerController;
	
	UPROPERTY()
	ETurningInPlace TurningInPlace;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;
	
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;
	
	UPROPERTY()
	FTimerHandle EliminatedTimer;

	UPROPERTY(EditDefaultsOnly)
	float EliminatedDelay = 2.f;

	UPROPERTY()
	bool bEliminated = false;
	
	UPROPERTY()
	float TurnThreshold = .5f;

	UPROPERTY()
	FRotator ProxyRotationLastFrame;

	UPROPERTY()
	FRotator ProxyRotation;

	UPROPERTY()
	float ProxyYaw;

	UPROPERTY()
	float TimeSinceLastMovementReplication;

#pragma region AIM OFFSET

public:
	
	UPROPERTY()
	FRotator StartingAimRotation;
	
private:

	UPROPERTY()
	float AO_Yaw;

	UPROPERTY()
	float AO_Pitch;
	
	UPROPERTY()
	float InterpAO_Yaw;

	UPROPERTY()
	bool bRotateRootBone;
	
	UFUNCTION()
	void AimOffset(float DeltaTime);

#pragma endregion
	
#pragma region MONTAGES
	
	UPROPERTY(EditAnywhere, Category = Montages)
	UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Montages)
	UAnimMontage* ReloadMontage;
	
	UPROPERTY(EditAnywhere, Category = Montages)
	UAnimMontage* EliminatedMontage;

	UPROPERTY(EditAnywhere, Category = Montages)
	UAnimMontage* ThrowGrenadeMontage;

	UPROPERTY(EditAnywhere, Category = Montages)
	UAnimMontage* SwapWeaponMontage;

public:

	bool bFinishedSwapping = false;
	
	UFUNCTION()
	void PlayFireMontage(bool bAiming);

	UFUNCTION()
	void PlayReloadMontage();
	
	UFUNCTION()
	void PlayElimMontage();

	UFUNCTION()
	void PlayThrowGrenadeMontage();

	UFUNCTION()
	void PlaySwapWeaponMontage();

#pragma endregion

#pragma region ELIMINATION

private:
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	USoundCue* ElimBotSound;

#pragma endregion

#pragma region DISSOLVE EFFECT

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	
	UPROPERTY()
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;
	
	// Dynamic instance that we can change at runtime
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance_0;

	// Material instance set on the Blueprint, used with the dynamic material instance 
	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* DissolveMaterialInstance_0;
	
	// Dynamic instance that we can change at runtime
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance_1;

	// Material instance set on the Blueprint, used with the dynamic material instance 
	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* DissolveMaterialInstance_1;

	// Dynamic instance that we can change at runtime
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance_2;

	// Material instance set on the Blueprint, used with the dynamic material instance 
	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* DissolveMaterialInstance_2;

	// Dynamic instance that we can change at runtime
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance_3;

	// Material instance set on the Blueprint, used with the dynamic material instance 
	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* DissolveMaterialInstance_3;

	// Dynamic instance that we can change at runtime
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance_5;

	// Material instance set on the Blueprint, used with the dynamic material instance 
	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* DissolveMaterialInstance_5;

	// Dynamic instance that we can change at runtime
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance_6;

	// Material instance set on the Blueprint, used with the dynamic material instance 
	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* DissolveMaterialInstance_6;

	// Dynamic instance that we can change at runtime
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance_7;

	// Material instance set on the Blueprint, used with the dynamic material instance 
	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* DissolveMaterialInstance_7;

	// Dynamic instance that we can change at runtime
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance_8;

	// Material instance set on the Blueprint, used with the dynamic material instance 
	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* DissolveMaterialInstance_8;

	// Dynamic instance that we can change at runtime
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance_9;

	// Material instance set on the Blueprint, used with the dynamic material instance 
	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* DissolveMaterialInstance_9;
	
	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	UFUNCTION()
	void StartDissolve();

#pragma endregion

#pragma region TAKEN DAMAGE

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health ,VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats")
	float Shield = 0.f;

	UPROPERTY(Replicated)
	float DamageDirection;

	UPROPERTY()
	FRotator DamageRotationDelta;

	UPROPERTY(EditAnywhere, Category = Montages)
	UAnimMontage* HitReactMontage;
	
	UFUNCTION()
	void OnRep_Health(float LastHealth);
	
	UFUNCTION()
	void OnRep_Shield(float LastShield);
	
public:

	UFUNCTION()
	void PlayHitReactMontage();
	
	UFUNCTION(NetMulticast, Unreliable)
	void MultiCastHit();
	
#pragma endregion

#pragma region LEAVING SESSION

private:
	bool bLeftGame = false;

public:
	FOnLeftGame OnLeftGame;
	
	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();


#pragma endregion

#pragma region GAINING THE LEAD

private:
	
	UPROPERTY(EditAnywhere)
	UNiagaraSystem* CrownSystem;

	UPROPERTY()
	UNiagaraComponent* CrownComponent;

public:

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

	FORCEINLINE UNiagaraComponent* GetCrownComponent() const { return CrownComponent; }

#pragma endregion

#pragma region TEAMS

	UPROPERTY()
	AOnlineShooterGameMode* OnlineShooterGameMode;

	UPROPERTY(EditAnywhere, Category = "Elimination")
	UMaterialInstance* OriginalMaterial;
	
	UPROPERTY(EditAnywhere, Category = "Elimination")
	UMaterialInstance* RedMaterial;
	
	UPROPERTY(EditAnywhere, Category = "Elimination")
	UMaterialInstance* RedDissolveMatInst;

	UPROPERTY(EditAnywhere, Category = "Elimination")
	UMaterialInstance* BlueMaterial;
	
	UPROPERTY(EditAnywhere, Category = "Elimination")
	UMaterialInstance* BlueDissolveMatInst;

public:

	UFUNCTION()
	void SetTeamColor(ETeam InTeam);

#pragma endregion

public:
	
	UPROPERTY(EditAnywhere, Category = "WeaponRotationCorrection") 
	float RightHandRotationRoll;
	
	UPROPERTY(EditAnywhere, Category = "WeaponRotationCorrection") 
	float RightHandRotationYaw;
	
	UPROPERTY(EditAnywhere, Category = "WeaponRotationCorrection") 
	float RightHandRotationPitch;

	UPROPERTY()
	AOnlineShooterPlayerState* OnlineShooterPlayerState;

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

#pragma region SERVER-SIDE REWIND BOXES

protected:

	UPROPERTY(EditAnywhere)
	UBoxComponent* head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_02;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_03;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_r;

public:

	UPROPERTY()
	TMap<FName, UBoxComponent*> HitCollisionBoxes;
	
#pragma endregion

private:
	
	UFUNCTION()
	void SimProxiesTurn();
	
	UFUNCTION() // Handles character turn in place animations depending on Z-axis offset.
	void TurnInPlace(float DeltaTime);
	
	UFUNCTION() 
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);
	
	UFUNCTION() // Hides meshes when camera too close
	void HideMesh();

	UFUNCTION()
	void EliminatedTimerFinished();

protected:
	
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);

	UFUNCTION()
	void RotateInPlace(float DeltaTime);
	
	// Poll for any relevant classes and initialize our HUD 
	UFUNCTION()
	void PollInit();

	
	UFUNCTION()
	void DropOrDestroyWeapons();

	UFUNCTION()
	void DropOrDestroyWeapon(AWeapon* Weapon);

public:
	
	UFUNCTION()
	void UpdateHUDHealth();

	UFUNCTION()
	void UpdateHUDShield();

	UFUNCTION()
	void UpdateHUDAmmo();

	UFUNCTION()
	void UpdateHUDGrenades();
	
	// Determines whether player overlap weapon
	void SetOverlappingWeapon(AWeapon* Weapon);

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsWeaponEquipped() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAiming() const;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);
	
	UFUNCTION()
	void SpawnDefaultWeapon();

	UFUNCTION()
	AWeapon* GetEquippedWeapon() const;
	
	UFUNCTION()
	FVector GetHitTarget() const;

	UFUNCTION()
	void Eliminated(bool bPlayerLeftGame);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Eliminated(bool bPlayerLeftGame);

	UFUNCTION()
	ECombatState GetCombatState() const;

#pragma region GETTERS & SETTERS

	// Components
	FORCEINLINE UCameraComponent* GetFollowCamera() const			{ return FollowCamera; }
	FORCEINLINE USpringArmComponent* GetCameraBoom() const			{ return CameraBoom; }
	FORCEINLINE UCombatComponent* GetCombatComponent() const		{ return Combat; }
	FORCEINLINE UBuffComponent* GetBuffComponent() const			{ return Buff; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const	{ return AttachedGrenade; }
	
	// Animations
	FORCEINLINE float GetAO_Yaw() const								{ return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const							{ return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const			{ return TurningInPlace; }
	FORCEINLINE bool ShouldRotateRootBone() const					{ return bRotateRootBone; }
	FORCEINLINE bool IsFiring() const								{ return Combat ? Combat->IsFiring() : false; }
	bool IsLocallyReloading() const;

	// States
	FORCEINLINE bool IsEliminated() const							{ return bEliminated; }
	FORCEINLINE bool GetDisableGameplay() const						{ return bDisableGameplay; }

	// Montages
	FORCEINLINE UAnimMontage* GetReloadMontage() const				{ return ReloadMontage; }
	
	// Health
	FORCEINLINE float GetHealth() const								{ return Health; }
	FORCEINLINE float GetMaxHealth() const							{ return MaxHealth; }
	FORCEINLINE void SetHealth(float NewShield)						{ Health = NewShield; }

	// Shield
	FORCEINLINE float GetShield() const								{ return Shield; }
	FORCEINLINE float GetMaxShield() const							{ return MaxShield; }
	FORCEINLINE void SetShield(float NewShieldAmount)				{ Shield = NewShieldAmount; }
	
	// Lag Compensation
	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const{ return LagCompensation; }
	
	
#pragma endregion


};
