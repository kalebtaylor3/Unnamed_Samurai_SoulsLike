// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/ALSBaseCharacter.h"
#include "AI/EnemyHeldWeaponBase.h"
#include "EnemyCombatComponent.generated.h"

class ACharacter;

UENUM(BlueprintType)
enum class EEnemyAIState : uint8
{
	Idle,
	Chasing,
	Attacking
};

UENUM(BlueprintType)
enum class EEnemyDodgeDirection : uint8
{
	Backward,
	Left,
	Right
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALSV4_CPP_API UEnemyCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyCombatComponent();

	void PerformAttack();
	bool CanAttackCurrentTarget() const;
	bool CanAttackTarget(const AActor* Target) const;

	UPROPERTY()
	FName EnemyUniqueID;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* DodgeMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|Dodge")
	UAnimMontage* LeftDodgeMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|Dodge")
	UAnimMontage* RightDodgeMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float DodgeTriggerRange = 200.0f; // Optional tweakable range

	UPROPERTY(EditAnywhere, Category = "Combat|Dodge", meta = (ClampMin = "0.0"))
	float BackwardDodgeWeight = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Combat|Dodge", meta = (ClampMin = "0.0"))
	float LeftDodgeWeight = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Combat|Dodge", meta = (ClampMin = "0.0"))
	float RightDodgeWeight = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Combat|Dodge|Manual Movement")
	bool bUseManualLateralDodgeMovement = true;

	UPROPERTY(EditAnywhere, Category = "Combat|Dodge|Manual Movement", meta = (ClampMin = "0.0", EditCondition = "bUseManualLateralDodgeMovement"))
	float ManualLateralDodgeDistance = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Combat|Dodge|Invincibility", meta = (ClampMin = "0.0"))
	float DodgeInvincibilityDelay = 0.08f;

	UPROPERTY(EditAnywhere, Category = "Combat|Dodge|Invincibility", meta = (ClampMin = "0.0"))
	float DodgeInvincibilityDuration = 0.22f;

	UPROPERTY(EditAnywhere, Category = "Combat|Kick")
	UAnimMontage* KickMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|Kick")
	float KickRange = 175.0f;

	UPROPERTY(EditAnywhere, Category = "Combat|Dodge", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BetweenAttackDodgeChance = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Combat|Dodge")
	float BetweenAttackDodgeRange = 220.0f;

	UPROPERTY(EditAnywhere, Category = "Combat|Dodge", meta = (ClampMin = "0.0"))
	float MinTimeBetweenAttackDodges = 1.25f;

	UPROPERTY(EditAnywhere, Category = "Combat|Dodge|On Hit")
	bool bEnablePostHitDodge = true;

	UPROPERTY(EditAnywhere, Category = "Combat|Dodge|On Hit", meta = (ClampMin = "0.0"))
	float PostHitDodgeDelay = 0.65f;

	UPROPERTY(EditAnywhere, Category = "Combat|Dodge|On Hit", meta = (ClampMin = "0.0"))
	float PostHitDodgeRange = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Combat|Dodge|On Hit", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PostHitDodgeChance = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Combat|Dodge|On Hit")
	bool bGuaranteePostHitDodgeWhenUnaware = true;

	UPROPERTY(EditAnywhere, Category = "Combat|Dodge|On Hit", meta = (ClampMin = "0.0"))
	float WasHitResetDelay = 0.9f;

	UPROPERTY(EditAnywhere, Category = "Combat|Kick", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BetweenAttackKickChance = 0.75f;

	void PlayDodgeMontage();
	bool TryPlayDodgeMontage();
	bool TryPlayDodgeMontage(EEnemyDodgeDirection DodgeDirection);
	bool HasDodgeMontage() const;
	UAnimMontage* GetActiveDodgeMontage() const { return ActiveDodgeMontage; }
	void OnDodgeFinished();
	bool TryPlayKickMontage();
	void OnKickFinished();
	bool CanKickCurrentTarget() const;
	void BeginKickDamageWindow();
	void TickKickDamageWindow(float DamageAmount, float HitRadius, float HitForwardOffset, float LaunchStrength, float LaunchUpwardStrength);
	void EndKickDamageWindow();
	void HandleOwnerDeath();
	void HandleOwnerHit(AActor* InstigatorActor);

	EEnemyAIState CurrentState = EEnemyAIState::Idle;

	FVector SpawnLocation;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<float> ComboStaminaCosts;

	FTimerHandle StaminaRegenTimer;

	UPROPERTY(EditAnywhere, Category = "Stamina")
	float MaxStamina = 100.f;

	UPROPERTY(EditAnywhere, Category = "Stamina")
	float StaminaRegenRate = 10.f; // Per second

	float CurrentStamina = 100.f;
	bool bComboOngoing = false;

	int32 ComboIndex = 0;
	int32 ComboStartIndex = 0;

	void ContinueCombo(); // new
	float GetCurrentStamina() const { return CurrentStamina; }
	void StartStaminaRegen();
	void RegenStamina();

	UPROPERTY(EditAnywhere, Category = "Weapon")
	AEnemyHeldWeaponBase* HeldWeaponActor;

	void FaceTargetOnce();

	bool bShouldRotateToTarget = false;
	float RotationInterpDuration = 0.15f;
	float RotationInterpRemainingTime = 0.f;
	FRotator RotationStart;
	FRotator RotationTarget;


protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	class AALSBaseCharacter* OwnerCharacter;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackRange = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<UAnimMontage*> AttackMontages;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float CooldownTime = 1.0f;

	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector TargetActorKey;

	UBlackboardComponent* Blackboard = nullptr;

	FTimerHandle CooldownTimer;
	FTimerHandle DodgeInvincibilityDelayTimer;
	FTimerHandle PostHitDodgeTimer;
	FTimerHandle WasHitResetTimer;

	int32 CurrentAttackIndex = 0;

	bool bIsAttacking = false;

	void TickIdle();
	void TickChasing();
	void TickAttacking();

	void EnterState(EEnemyAIState NewState);
	void OnAttackFinished();
	void StartManualLateralDodgeMovement(EEnemyDodgeDirection DodgeDirection, float DodgeDuration);
	void TickManualLateralDodgeMovement(float DeltaTime);
	void StopManualLateralDodgeMovement();
	void StartKickKnockback(ACharacter* HitCharacter, const FVector& Direction, float LaunchStrength);
	void TickKickKnockback(float DeltaTime);
	void StopKickKnockback();
	bool ShouldDodgeBetweenAttacks(const AActor* Target) const;
	bool ShouldKickBetweenAttacks(const AActor* Target) const;
	void RequestDodge();
	void BeginDodgeInvincibility();
	void TryPostHitDodge();
	void ClearWasHitFlag();
	EEnemyDodgeDirection ChooseDodgeDirection() const;
	UAnimMontage* GetDodgeMontageForDirection(EEnemyDodgeDirection DodgeDirection) const;

	float LastBetweenAttackDodgeTime = -1000.0f;
	bool bForceNextPostHitDodge = false;
	UPROPERTY()
	UAnimMontage* ActiveDodgeMontage = nullptr;
	bool bIsManualLateralDodgeActive = false;
	FVector ManualLateralDodgeDirection = FVector::ZeroVector;
	float ManualLateralDodgeElapsedTime = 0.0f;
	float ManualLateralDodgeDuration = 0.0f;
	float ManualLateralDodgePreviousAlpha = 0.0f;
	TSet<AActor*> KickHitActors;
	UPROPERTY()
	ACharacter* KickKnockbackTarget = nullptr;
	FVector KickKnockbackDirection = FVector::ZeroVector;
	float KickKnockbackDistance = 0.0f;
	float KickKnockbackDuration = 0.25f;
	float KickKnockbackElapsedTime = 0.0f;
	float KickKnockbackPreviousAlpha = 0.0f;
};
