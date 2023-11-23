#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

// References
class AWeapon;
class AOnlineShooterCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ONLINESHOOTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UCombatComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	// Friend Classes
	friend AOnlineShooterCharacter;

protected:
	
	virtual void BeginPlay() override;

private:

	UPROPERTY()
	AWeapon* EquippedWeapon;

	UPROPERTY()
	AOnlineShooterCharacter* Character;

	UFUNCTION()
	void EquipWeapon(AWeapon* WeaponToEquip);

public:

	
	
	
	


	
	
};
