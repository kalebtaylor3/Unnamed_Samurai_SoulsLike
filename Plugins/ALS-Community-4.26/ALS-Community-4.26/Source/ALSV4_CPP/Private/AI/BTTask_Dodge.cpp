// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_Dodge.h"
#include "AIController.h"
#include "Character/ALSBaseCharacter.h"
#include "AI/EnemyCombatComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_Dodge::UBTTask_Dodge()
{
	NodeName = "Dodge";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_Dodge::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		if (AALSBaseCharacter* Enemy = Cast<AALSBaseCharacter>(AIController->GetPawn()))
		{
			CombatComponent = Enemy->FindComponentByClass<UEnemyCombatComponent>();

			if (CombatComponent && CombatComponent->DodgeMontage)
			{
				CombatComponent->PlayDodgeMontage();

				DodgeDuration = CombatComponent->DodgeMontage->GetPlayLength();
				ElapsedTime = 0.f;
				bIsWaitingForMontageEnd = true;

				return EBTNodeResult::InProgress;
			}
		}
	}

	return EBTNodeResult::Failed;
}

void UBTTask_Dodge::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!bIsWaitingForMontageEnd || !CombatComponent)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ElapsedTime += DeltaSeconds;

	if (ElapsedTime >= DodgeDuration)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);

		if (OwnerComp.GetBlackboardComponent())
		{
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("ShouldDodge", false);
		}
	}
}
