// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTDecorator_NotEnoughStamina.h"
#include "AI/EnemyCombatComponent.h"
#include "AIController.h"

UBTDecorator_NotEnoughStamina::UBTDecorator_NotEnoughStamina()
{
	NodeName = "Not Enough Stamina";
}

bool UBTDecorator_NotEnoughStamina::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	APawn* AIPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (!AIPawn) return false;

	UEnemyCombatComponent* CombatComp = AIPawn->FindComponentByClass<UEnemyCombatComponent>();
	if (!CombatComp) return false;

	const bool bNotEnoughStamina = CombatComp->GetCurrentStamina() < RequiredStamina;
	if (bNotEnoughStamina)
	{
		CombatComp->StartStaminaRegen();
	}

	return bNotEnoughStamina;
}
