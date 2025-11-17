// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTDecorator_HasStamina.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Character/ALSBaseCharacter.h"
#include "AI/EnemyCombatComponent.h"

UBTDecorator_HasStamina::UBTDecorator_HasStamina()
{
	NodeName = "Has Enough Stamina";
	TargetActorKey.SelectedKeyName = "TargetActor";
}

bool UBTDecorator_HasStamina::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return false;

	AALSBaseCharacter* AICharacter = Cast<AALSBaseCharacter>(AIController->GetPawn());
	if (!AICharacter) return false;

	UEnemyCombatComponent* CombatComponent = AICharacter->FindComponentByClass<UEnemyCombatComponent>();
	if (!CombatComponent) return false;

	return CombatComponent->GetCurrentStamina() >= RequiredStamina;
}

