#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <Weapons/WeaponBase.h>
#include "CombatComponent.generated.h"


UENUM(BlueprintType)
enum class ECombatStance : uint8
{
	OneHanded  UMETA(DisplayName = "One-Handed"),
	TwoHanded  UMETA(DisplayName = "Two-Handed")
	// Add more stances later like DualWield, Magic, etc.
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALSV4_CPP_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	void LightAttack();
	void HeavyAttackCheck(bool bValue);
	void StartChargeHeavyAttack();
	void ReleaseChargeHeavyAttack();

	void OnAttackStarted();
	void OnComboWindowOpened();
	void OnComboWindowClosed();
	void OnAttackEnded();
	void SetCheckingForStanceChange(bool value);
	void SetStance();
	void EnableStanceChange();

	bool bIsAttacking = false;
	bool canRoll = true;
	bool checkingForStanceChange = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	ECombatStance CurrentStance = ECombatStance::OneHanded;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	UWeaponBase* CurrentWeapon;

	bool bIsChargingHeavy = false;
	bool bIsHoldingCharge = false;
	bool bIsLoopingCharge = false;
	bool bCanChangeStance = true;
	bool bCanReceiveInput = true;
	bool bInputQueuedThisWindow = false;
	TQueue<int32> QueuedComboIndices;

	UFUNCTION()
	void InterruptAttack();

	UFUNCTION(BlueprintCallable)
	void UseAshOfWar();

protected:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	class AALSBaseCharacter* OwnerCharacter;

	int32 QueuedCount = 0;
	int32 AttackIndex = 0;
	bool bIsChargeHoldPlaying = false;
	bool bHasJumpedToLoop = false;

	FTimerHandle ChargeLoopTimer;
	FTimerHandle StanceChangeCooldownTimer;

	void PlayChargeLoopMontage();
	void PlayLightAttackMontage(int32 Index, const TArray<UAnimMontage*>& MontageList);
};
