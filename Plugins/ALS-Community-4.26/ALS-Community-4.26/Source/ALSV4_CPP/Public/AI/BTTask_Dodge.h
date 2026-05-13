// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Dodge.generated.h"

class AALSBaseCharacter;

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

	UPROPERTY(EditAnywhere, Category = "Dodge")
	bool bRequireShouldDodge = true;

	UPROPERTY(EditAnywhere, Category = "Dodge")
	bool bAllowProximityDodge = false;

	UPROPERTY(EditAnywhere, Category = "Dodge", meta = (EditCondition = "bAllowProximityDodge"))
	float ProximityDodgeRange = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Dodge", meta = (EditCondition = "bAllowProximityDodge", ClampMin = "0.0", ClampMax = "1.0"))
	float ProximityDodgeChance = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Dodge", meta = (ClampMin = "0.0"))
	float MinimumTimeBetweenDodges = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Dodge")
	bool bSucceedWhenSkippingDodge = false;

private:
	bool ShouldAttemptDodge(UBehaviorTreeComponent& OwnerComp, AALSBaseCharacter* Enemy) const;

	bool bIsWaitingForMontageEnd = false;
	float DodgeDuration = 0.0f;
	float ElapsedTime = 0.0f;
	float LastDodgeTime = -1000.0f;
	
};
