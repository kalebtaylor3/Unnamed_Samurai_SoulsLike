#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_WolfCirclePlayer.generated.h"

class UEnemyWolfCombatComponent;

UCLASS(Category = ALS, meta = (DisplayName = "Wolf Circle Player"))
class ALSV4_CPP_API UBTTask_WolfCirclePlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_WolfCirclePlayer();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY()
	float DesiredDistanceFromTarget = 330.0f;

	UPROPERTY(EditAnywhere, Category = "Circle", meta = (ClampMin = "0.0"))
	float MinDesiredDistanceFromTarget = 330.0f;

	UPROPERTY(EditAnywhere, Category = "Circle", meta = (ClampMin = "0.0"))
	float MaxDesiredDistanceFromTarget = 460.0f;

	UPROPERTY(EditAnywhere, Category = "Circle", meta = (ClampMin = "0.0"))
	float SideStepDistance = 300.0f;

	UPROPERTY()
	float CircleDuration = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Circle", meta = (ClampMin = "0.0"))
	float MinCircleDuration = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Circle", meta = (ClampMin = "0.0"))
	float MaxCircleDuration = 1.25f;

	UPROPERTY(EditAnywhere, Category = "Circle", meta = (ClampMin = "0.1"))
	float MaxTravelTime = 4.0f;

	UPROPERTY(EditAnywhere, Category = "Circle")
	float AcceptanceRadius = 35.0f;

private:
	bool bCircleClockwise = true;
	bool bHoldingAtCirclePoint = false;
	bool bCircleWarningStarted = false;
	FVector ActiveDesiredLocation = FVector::ZeroVector;
	float ActiveTravelTime = 0.0f;
	float ActiveHoldTime = 0.0f;
	float ActiveHoldDuration = 0.0f;

	UPROPERTY()
	UEnemyWolfCombatComponent* ActiveWolfCombat = nullptr;

	void FinishCircleTask(UBehaviorTreeComponent& OwnerComp, EBTNodeResult::Type Result);
	void BeginCircleHold(UBehaviorTreeComponent& OwnerComp);
};
