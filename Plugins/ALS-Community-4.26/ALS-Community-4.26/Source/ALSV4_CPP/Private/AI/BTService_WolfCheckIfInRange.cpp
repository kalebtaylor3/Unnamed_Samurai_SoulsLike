#include "AI/BTService_WolfCheckIfInRange.h"

#include "AIController.h"
#include "AI/EnemyWolfCombatComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"

UBTService_WolfCheckIfInRange::UBTService_WolfCheckIfInRange()
{
	NodeName = "Wolf Check If In Attack Range";
	bNotifyTick = true;

	TargetActorKey.SelectedKeyName = "TargetActor";
	IsInRangeKey.SelectedKeyName = "IsInAttackRange";
}

void UBTService_WolfCheckIfInRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* AIPawn = AIController ? AIController->GetPawn() : nullptr;
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!AIPawn || !Blackboard)
	{
		return;
	}

	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Target)
	{
		Blackboard->SetValueAsBool(IsInRangeKey.SelectedKeyName, false);
		return;
	}

	if (const UEnemyWolfCombatComponent* WolfCombat = AIPawn->FindComponentByClass<UEnemyWolfCombatComponent>())
	{
		const float DistanceFromSpawn = FVector::Dist2D(AIPawn->GetActorLocation(), WolfCombat->GetSpawnLocation());
		if (DistanceFromSpawn > WolfCombat->LeashRadius)
		{
			AIController->ClearFocus(EAIFocusPriority::Gameplay);
			Blackboard->SetValueAsBool(IsInRangeKey.SelectedKeyName, false);
			Blackboard->SetValueAsBool("FirstTimeSeeingPlayer", false);
			Blackboard->SetValueAsBool("WasHit", false);
			Blackboard->SetValueAsBool("ShouldCircle", false);
			Blackboard->ClearValue(TargetActorKey.SelectedKeyName);
			return;
		}
	}

	const float DistanceToTarget = FVector::Dist2D(AIPawn->GetActorLocation(), Target->GetActorLocation());
	const bool bWasInRange = Blackboard->GetValueAsBool(IsInRangeKey.SelectedKeyName);
	const float RangeToUse = bWasInRange ? AttackRange + AttackRangeExitBuffer : AttackRange;
	Blackboard->SetValueAsBool(IsInRangeKey.SelectedKeyName, DistanceToTarget <= RangeToUse);
}
