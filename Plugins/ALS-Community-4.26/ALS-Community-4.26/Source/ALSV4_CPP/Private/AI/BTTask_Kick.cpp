// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_Kick.h"
#include "AIController.h"
#include "AI/EnemyCombatComponent.h"
#include "Character/ALSBaseCharacter.h"

UBTTask_Kick::UBTTask_Kick()
{
	NodeName = "Kick Player";
	bNotifyTick = true;
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_Kick::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AALSBaseCharacter* Enemy = Cast<AALSBaseCharacter>(AIController->GetPawn());
	if (!Enemy)
	{
		return EBTNodeResult::Failed;
	}

	CachedCombatComponent = Enemy->FindComponentByClass<UEnemyCombatComponent>();
	if (!CachedCombatComponent || !CachedCombatComponent->TryPlayKickMontage())
	{
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_Kick::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!CachedCombatComponent)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (CachedCombatComponent->CurrentState != EEnemyAIState::Attacking)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
