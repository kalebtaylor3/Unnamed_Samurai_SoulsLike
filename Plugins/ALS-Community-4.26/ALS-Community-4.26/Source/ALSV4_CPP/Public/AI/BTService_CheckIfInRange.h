// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckIfInRange.generated.h"

/**
 * 
 */
UCLASS(Category = ALS, meta = (DisplayName = "Check If In Range"))
class ALSV4_CPP_API UBTService_CheckIfInRange : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_CheckIfInRange();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsInRangeKey;

	UPROPERTY(EditAnywhere, Category = "Settings")
	float AttackRange = 250.0f; // Elden Ring-like melee range

	UPROPERTY(EditAnywhere, Category = "Leash")
	float LeashRadius = 1500.f;
};
