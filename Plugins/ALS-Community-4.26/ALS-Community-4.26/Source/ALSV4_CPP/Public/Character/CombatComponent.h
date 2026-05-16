#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <Weapons/WeaponBase.h>
#include "CombatComponent.generated.h"

class UMagicWeaponBase;
class USpellBase;

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
	void StartBowDraw();
	void CancelBowDraw();
	UFUNCTION(BlueprintCallable, Category = "Magic")
	void CastBaseMagic();

	UFUNCTION(BlueprintCallable, Category = "Magic")
	void CastEquippedSpell();

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
	bool bIsDrawingBow = false;
	bool bCanFireDrawnBow = false;
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
	FTimerHandle BowDrawReadyTimer;
	FTimerHandle StanceChangeCooldownTimer;
	FTimerHandle MagicCastEndTimer;
	FTimerHandle MagicSpellReleaseTimer;

	void PlayChargeLoopMontage();
	void FinishBowDraw();
	bool IsBowEquipped() const;
	bool IsMagicWeaponEquipped() const;
	bool SpendMagicCosts(float FPCost, float StaminaCost) const;
	float PlayMagicCastMontage(UAnimMontage* Montage);
	void BeginMagicCast(float CastDuration);
	void FinishMagicCast();
	void ReleaseBaseMagic(UMagicWeaponBase* MagicWeapon);
	void ReleaseEquippedSpell(UMagicWeaponBase* MagicWeapon, USpellBase* Spell);
	bool FireBow();
	void ShowBowPreviewArrow();
	void HideBowPreviewArrow();
	FVector GetBowArrowSpawnLocation() const;
	FRotator GetBowAimRotation(const FVector& SpawnLocation) const;
	void PlayLightAttackMontage(int32 Index, const TArray<UAnimMontage*>& MontageList);
};
