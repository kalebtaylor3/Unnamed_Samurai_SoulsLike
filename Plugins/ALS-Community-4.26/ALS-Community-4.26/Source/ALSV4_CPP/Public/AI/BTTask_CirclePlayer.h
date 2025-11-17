// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_CirclePlayer.generated.h"

UCLASS()
class ALSV4_CPP_API UBTTask_CirclePlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_CirclePlayer();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Circle")
	float CircleRadius = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Circle")
	float CircleSpeed = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Circle")
	float CircleDuration = 1.0f; // How long to circle before re-evaluating BT

	UPROPERTY()
	bool bCircleClockwise = true;
};