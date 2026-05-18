#include "AI/BTTask_WolfAttackPlayer.h"

#include "AIController.h"
#include "AI/EnemyWolfCombatComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_WolfAttackPlayer::UBTTask_WolfAttackPlayer()
{
	NodeName = "Wolf Attack Player";
	bNotifyTick = true;
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_WolfAttackPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* AIPawn = AIController ? AIController->GetPawn() : nullptr;
	if (!AIPawn)
	{
		return EBTNodeResult::Failed;
	}

	CachedCombatComponent = AIPawn->FindComponentByClass<UEnemyWolfCombatComponent>();
	if (!CachedCombatComponent)
	{
		return EBTNodeResult::Failed;
	}

	if (UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent())
	{
		if (Blackboard->GetValueAsBool("ShouldCircle"))
		{
			return EBTNodeResult::Failed;
		}
	}

	if (!CachedCombatComponent->CanAttackCurrentTarget())
	{
		if (UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent())
		{
			Blackboard->SetValueAsBool("IsInAttackRange", false);
		}

		return EBTNodeResult::Failed;
	}

	CachedCombatComponent->PerformAttack();
	return CachedCombatComponent->IsAttackInProgress() ? EBTNodeResult::InProgress : EBTNodeResult::Failed;
}

void UBTTask_WolfAttackPlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!CachedCombatComponent)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (!CachedCombatComponent->IsAttackInProgress())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
