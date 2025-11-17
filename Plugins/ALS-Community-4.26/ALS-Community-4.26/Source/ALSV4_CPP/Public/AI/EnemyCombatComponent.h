// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/ALSBaseCharacter.h"
#include "AI/EnemyHeldWeaponBase.h"
#include "EnemyCombatComponent.generated.h"

UENUM(BlueprintType)
enum class EEnemyAIState : uint8
{
	Idle,
	Chasing,
	Attacking
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALSV4_CPP_API UEnemyCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyCombatComponent();

	void PerformAttack();

	UPROPERTY()
	FName EnemyUniqueID;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* DodgeMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float DodgeTriggerRange = 200.0f; // Optional tweakable range

	void PlayDodgeMontage();
	void OnDodgeFinished();

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

	int32 CurrentAttackIndex = 0;

	bool bIsAttacking = false;

	void TickIdle();
	void TickChasing();
	void TickAttacking();

	void EnterState(EEnemyAIState NewState);
	void OnAttackFinished();
};