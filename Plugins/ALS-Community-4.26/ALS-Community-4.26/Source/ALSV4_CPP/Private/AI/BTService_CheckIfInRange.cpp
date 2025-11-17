// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_CheckIfInRange.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include <AI/EnemyCombatComponent.h>

UBTService_CheckIfInRange::UBTService_CheckIfInRange()
{
	NodeName = "Check If In Attack Range";
	bNotifyBecomeRelevant = true;
	bNotifyTick = true;

	TargetActorKey.SelectedKeyName = "TargetActor";
	IsInRangeKey.SelectedKeyName = "IsInAttackRange";
}

void UBTService_CheckIfInRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* AIPawn = AIController ? AIController->GetPawn() : nullptr;
	if (!AIPawn) return;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Target)
	{
		Blackboard->SetValueAsBool(IsInRangeKey.SelectedKeyName, false);
		return;
	}

	// Distance to target
	const float DistanceToTarget = FVector::Dist2D(AIPawn->GetActorLocation(), Target->GetActorLocation());

	// Leash check
	UEnemyCombatComponent* CombatComp = AIPawn->FindComponentByClass<UEnemyCombatComponent>();
	if (CombatComp)
	{
		const float DistanceFromSpawn = FVector::Dist2D(AIPawn->GetActorLocation(), Target->GetActorLocation());
		if (DistanceFromSpawn > LeashRadius)
		{
			// Exceeded leash: stop combat logic
			OwnerComp.GetAIOwner()->SetFocus(nullptr);
			OwnerComp.GetAIOwner()->GetDesiredRotation();
			Blackboard->SetValueAsBool(IsInRangeKey.SelectedKeyName, false);
			Blackboard->ClearValue(TargetActorKey.SelectedKeyName);
			Blackboard->SetValueAsBool("FirstTimeSeeingPlayer", false);
			Blackboard->GetBrainComponent()->GetAIOwner()->SetFocus(nullptr);
			return;
		}
	}

	// Set attack range key
	const bool bInRange = DistanceToTarget <= AttackRange;
	Blackboard->SetValueAsBool(IsInRangeKey.SelectedKeyName, bInRange);
}

