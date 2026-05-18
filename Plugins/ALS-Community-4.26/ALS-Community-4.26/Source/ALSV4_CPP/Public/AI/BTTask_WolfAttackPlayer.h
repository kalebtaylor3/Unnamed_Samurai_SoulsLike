#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_WolfAttackPlayer.generated.h"

class UEnemyWolfCombatComponent;

UCLASS(Category = ALS, meta = (DisplayName = "Wolf Attack Player"))
class ALSV4_CPP_API UBTTask_WolfAttackPlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_WolfAttackPlayer();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY()
	UEnemyWolfCombatComponent* CachedCombatComponent = nullptr;
};
