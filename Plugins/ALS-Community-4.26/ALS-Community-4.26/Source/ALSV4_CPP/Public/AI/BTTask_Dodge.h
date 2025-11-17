// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Dodge.generated.h"

/**
 * 
 */
UCLASS()
class ALSV4_CPP_API UBTTask_Dodge : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Dodge();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnGameplayTaskActivated(UGameplayTask& Task) override {}

protected:
	UPROPERTY()
	class UEnemyCombatComponent* CombatComponent;

private:
	bool bIsWaitingForMontageEnd = false;
	float DodgeDuration = 0.0f;
	float ElapsedTime = 0.0f;
	
};
