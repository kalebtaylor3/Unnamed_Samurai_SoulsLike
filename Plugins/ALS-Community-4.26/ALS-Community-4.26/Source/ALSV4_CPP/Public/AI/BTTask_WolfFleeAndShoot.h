#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_WolfFleeAndShoot.generated.h"

class UEnemyWolfCombatComponent;

UCLASS(Category = ALS, meta = (DisplayName = "Wolf Flee And Shoot"))
class ALSV4_CPP_API UBTTask_WolfFleeAndShoot : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_WolfFleeAndShoot();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Flee", meta = (ClampMin = "0.0"))
	float ClearDistanceTolerance = 75.0f;

	UPROPERTY(EditAnywhere, Category = "Flee", meta = (ClampMin = "0.1"))
	float MaxFleeTravelTime = 4.0f;

	UPROPERTY(EditAnywhere, Category = "Flee", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float EscapeAngleRandomnessDegrees = 45.0f;

	UPROPERTY(EditAnywhere, Category = "Flee", meta = (ClampMin = "0.0"))
	float RepositionDistanceBuffer = 125.0f;

	UPROPERTY(EditAnywhere, Category = "Shoot", meta = (ClampMin = "0.0"))
	float PostShotHoldTime = 0.15f;

private:
	enum class EWolfFleeShootPhase : uint8
	{
		MovingAway,
		FacingPlayer,
		Shooting
	};

	EWolfFleeShootPhase Phase = EWolfFleeShootPhase::MovingAway;
	FVector ActiveFleeLocation = FVector::ZeroVector;
	float ActiveElapsedTime = 0.0f;
	float ActiveShootDuration = 0.0f;
	bool bShotMontageStarted = false;

	UPROPERTY()
	UEnemyWolfCombatComponent* ActiveWolfCombat = nullptr;

	bool StartFleeMove(UBehaviorTreeComponent& OwnerComp);
	void BeginShootFacingPhase(UBehaviorTreeComponent& OwnerComp);
	void BeginShotMontagePhase();
	void FinishFleeShootTask(UBehaviorTreeComponent& OwnerComp, EBTNodeResult::Type Result);
	void CleanupActiveTask();
};
