// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_AttackPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/ALSBaseCharacter.h"
#include "AI/EnemyCombatComponent.h"

UBTTask_AttackPlayer::UBTTask_AttackPlayer()
{
	NodeName = "Attack Player";
	bNotifyTick = true;
	bNotifyTaskFinished = true; // Important!
	TargetActorKey.SelectedKeyName = "TargetActor";
}

EBTNodeResult::Type UBTTask_AttackPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	AALSBaseCharacter* AICharacter = Cast<AALSBaseCharacter>(AIController->GetPawn());
	if (!AICharacter) return EBTNodeResult::Failed;

	CachedCombatComponent = AICharacter->FindComponentByClass<UEnemyCombatComponent>();
	if (!CachedCombatComponent) return EBTNodeResult::Failed;

	CachedCombatComponent->PerformAttack();

	return EBTNodeResult::InProgress;
}

void UBTTask_AttackPlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!CachedCombatComponent)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (CachedCombatComponent->CurrentState != EEnemyAIState::Attacking)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}