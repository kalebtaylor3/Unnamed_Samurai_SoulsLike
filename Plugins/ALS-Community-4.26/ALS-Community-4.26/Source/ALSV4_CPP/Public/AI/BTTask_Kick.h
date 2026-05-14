// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Kick.generated.h"

class UEnemyCombatComponent;

UCLASS(Category = ALS, meta = (DisplayName = "Kick Player"))
class ALSV4_CPP_API UBTTask_Kick : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Kick();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY()
	UEnemyCombatComponent* CachedCombatComponent = nullptr;
};
