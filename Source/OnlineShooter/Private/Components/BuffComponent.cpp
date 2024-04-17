#include "Components/BuffComponent.h"

#include "NiagaraComponent.h"
#include "Characters/OnlineShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
}

// Pickup Heal
void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}
void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || !Character || Character->IsEliminated()) return;

	const float HealThisFrame = HealingRate * DeltaTime;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));
	Character->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;

	if (AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

// Pickup Shield 
void UBuffComponent::ReplenishShield(float ShieldAmount, float ReplenishTime)
{
	bReplenishShield = true;
	ShieldReplenishRate = ShieldAmount / ReplenishTime;
	ShieldReplenishAmount += ShieldAmount;
}
void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if (!bReplenishShield || !Character || Character->IsEliminated()) return;

	const float ReplenishThisFrame = ShieldReplenishRate * DeltaTime;
	Character->SetShield(FMath::Clamp(Character->GetShield() + ReplenishThisFrame, 0.f, Character->GetMaxShield()));
	Character->UpdateHUDShield();
	ShieldReplenishAmount -= ReplenishThisFrame;
	
	if (ShieldReplenishAmount <= 0.f || Character->GetShield() >= Character->GetMaxShield()) 
	{
		bReplenishShield = false;
		ShieldReplenishAmount = 0.f;
	}
}

// Pickup Speed
void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed) 
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}
void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float Time)
{	
	if(!Character) return;

	Character->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&UBuffComponent::ResetSpeeds,
		Time
		);

	if(Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed += BuffBaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = 600.f ;
	}

	Multicast_UpdateSpeeds(BuffBaseSpeed, BuffCrouchSpeed);
}
void UBuffComponent::ResetSpeeds() 
{
	if(!Character || !Character->GetCharacterMovement()) return;
	
	Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
	
	Multicast_UpdateSpeeds(InitialBaseSpeed, InitialCrouchSpeed);
}
void UBuffComponent::Multicast_UpdateSpeeds_Implementation(float BaseSpeed, float CrouchSpeed)
{
	Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
}

// Pickup Jump
void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
	InitialJumpVelocity = Velocity;
}
void UBuffComponent::BuffJump(float BuffJumpVelocity, float Time)
{
	if(!Character) return;

	Character->GetWorldTimerManager().SetTimer(
		JumpBuffTimer,
		this,
		&UBuffComponent::ResetJump,
		Time
		);

	if(Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
	}

	Multicast_UpdateJumpVelocity(BuffJumpVelocity);
}
void UBuffComponent::Multicast_UpdateJumpVelocity_Implementation(float NewJumpVelocity)
{
	Character->GetCharacterMovement()->JumpZVelocity = NewJumpVelocity;

	if (Character->GetCharacterMovement()->JumpZVelocity == InitialJumpVelocity && BuffEffect)
	{
		BuffEffect->Deactivate();
	}
}
void UBuffComponent::ResetJump()
{
	if(!Character || !Character->GetCharacterMovement()) return;
	
	Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;

	if (BuffEffect)
	{
		BuffEffect->Deactivate();
	}
	
	
	Multicast_UpdateJumpVelocity(InitialJumpVelocity);
}


