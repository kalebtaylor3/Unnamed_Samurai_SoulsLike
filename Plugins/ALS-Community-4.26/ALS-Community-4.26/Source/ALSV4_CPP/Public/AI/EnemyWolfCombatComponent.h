#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyWolfCombatComponent.generated.h"

class ACharacter;
class UAnimMontage;

UENUM(BlueprintType)
enum class EWolfAIState : uint8
{
	Idle,
	Chasing,
	Circling,
	Attacking
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALSV4_CPP_API UEnemyWolfCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyWolfCombatComponent();

	UFUNCTION(BlueprintCallable, Category = "Wolf Combat")
	void PerformAttack();

	UFUNCTION(BlueprintCallable, Category = "Wolf Combat")
	bool CanAttackCurrentTarget() const;

	UFUNCTION(BlueprintCallable, Category = "Wolf Combat")
	bool CanAttackTarget(const AActor* Target) const;

	UFUNCTION(BlueprintCallable, Category = "Wolf Combat")
	bool CanCircleCurrentTarget() const;

	UFUNCTION(BlueprintCallable, Category = "Wolf Combat")
	void BeginCircle();

	UFUNCTION(BlueprintCallable, Category = "Wolf Combat")
	void BeginCircleHold();

	UFUNCTION(BlueprintCallable, Category = "Wolf Combat")
	void FinishCircle();

	UFUNCTION(BlueprintCallable, Category = "Wolf Combat")
	void FaceTargetOnce();

	UFUNCTION(BlueprintCallable, Category = "Wolf Combat")
	void FaceTargetImmediately();

	UFUNCTION(BlueprintCallable, Category = "Wolf Combat")
	void BeginBiteDamageWindow(float DamageAmount, float HitRadius, float HitForwardOffset);

	UFUNCTION(BlueprintCallable, Category = "Wolf Combat")
	void TickBiteDamageWindow(float DamageAmount, float HitRadius, float HitForwardOffset);

	UFUNCTION(BlueprintCallable, Category = "Wolf Combat")
	void EndBiteDamageWindow();

	void HandleOwnerHit(AActor* InstigatorActor);
	void HandleOwnerDeath();
	void ClearWasHitFlag();
	void ClearShouldCircleFlag();
	bool IsFacingTargetForCircleWarning() const;
	float PlayCircleWarningMontage();
	void StopCircleWarningMontage();

	bool IsAttackInProgress() const { return bIsAttacking; }
	EWolfAIState GetCurrentState() const { return CurrentState; }
	FVector GetSpawnLocation() const { return SpawnLocation; }

	UPROPERTY(BlueprintReadOnly, Category = "Wolf Combat")
	EWolfAIState CurrentState = EWolfAIState::Idle;

	UPROPERTY(EditAnywhere, Category = "Wolf Combat")
	float AttackRange = 170.0f;

	UPROPERTY(EditAnywhere, Category = "Wolf Combat")
	float AttackRecoveryTime = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Wolf Combat")
	TArray<UAnimMontage*> AttackMontages;

	UPROPERTY(EditAnywhere, Category = "Wolf Combat|Movement")
	float ChaseWalkSpeed = 520.0f;

	UPROPERTY(EditAnywhere, Category = "Wolf Combat|Movement")
	float CircleWalkSpeed = 360.0f;

	UPROPERTY(EditAnywhere, Category = "Wolf Combat|Movement")
	float AttackWalkSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Wolf Combat|Circle", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CircleAfterAttackChance = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Wolf Combat|Circle", meta = (ClampMin = "0.0"))
	float MinTimeBetweenCircles = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Wolf Combat|Circle")
	float MinCircleRange = 160.0f;

	UPROPERTY(EditAnywhere, Category = "Wolf Combat|Circle")
	float MaxCircleRange = 650.0f;

	UPROPERTY(EditAnywhere, Category = "Wolf Combat|Circle")
	float CircleFaceTargetInterpSpeed = 12.0f;

	UPROPERTY(EditAnywhere, Category = "Wolf Combat|Circle")
	UAnimMontage* CircleWarningMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Wolf Combat|Circle", meta = (ClampMin = "1.0", ClampMax = "180.0"))
	float CircleWarningFacingAngleTolerance = 12.0f;

	UPROPERTY(EditAnywhere, Category = "Wolf Combat|Circle", meta = (ClampMin = "0.1"))
	float CircleWarningMontagePlayRate = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Wolf Combat|Circle", meta = (ClampMin = "0.0"))
	float CircleWarningMontageBlendOutTime = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Wolf Combat|Hit React")
	float WasHitResetDelay = 0.9f;

	UPROPERTY(EditAnywhere, Category = "Wolf Combat|Leash")
	float LeashRadius = 4000.0f;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	ACharacter* OwnerCharacter = nullptr;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName IsInAttackRangeKeyName = TEXT("IsInAttackRange");

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName WasHitKeyName = TEXT("WasHit");

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName ShouldCircleKeyName = TEXT("ShouldCircle");

	UPROPERTY()
	UBlackboardComponent* Blackboard = nullptr;

	FTimerHandle AttackFinishedTimer;
	FTimerHandle WasHitResetTimer;

	UPROPERTY()
	UAnimMontage* ActiveAttackMontage = nullptr;

	FVector SpawnLocation = FVector::ZeroVector;
	TSet<AActor*> BiteHitActors;
	int32 LastAttackIndex = INDEX_NONE;
	bool bIsAttacking = false;
	bool bShouldRotateToTarget = false;
	float RotationInterpDuration = 0.12f;
	float RotationInterpRemainingTime = 0.0f;
	FRotator RotationStart = FRotator::ZeroRotator;
	FRotator RotationTarget = FRotator::ZeroRotator;
	float LastCircleTime = -1000.0f;
	float SavedWalkSpeed = 0.0f;
	bool bSavedOrientRotationToMovement = true;
	bool bSavedUseControllerDesiredRotation = false;
	bool bSavedUseControllerRotationYaw = false;
	bool bHasSavedCircleRotationSettings = false;
	bool bFaceTargetWhileCircling = false;

	void EnterState(EWolfAIState NewState);
	void OnAttackFinished();
	UAnimMontage* ChooseAttackMontage();
	bool ShouldCircleAfterAttack(const AActor* Target) const;
	void RequestCircle();
	void FaceTargetDuringCircle(float DeltaTime);
	void BeginCircleFacingLock();
	void EndCircleFacingLock();
	void RestoreMovementAfterAction();
	void ApplyWalkSpeed(float NewWalkSpeed);
	bool IsDead() const;
};
