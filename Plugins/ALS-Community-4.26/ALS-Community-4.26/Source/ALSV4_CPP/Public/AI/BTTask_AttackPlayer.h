// BTTask_AttackPlayer.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "EnemyCombatComponent.h"
#include "BTTask_AttackPlayer.generated.h"

UCLASS(Category = ALS, meta = (DisplayName = "Attack Player"))
class ALSV4_CPP_API UBTTask_AttackPlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_AttackPlayer();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UEnemyCombatComponent* CachedCombatComponent = nullptr;
};
