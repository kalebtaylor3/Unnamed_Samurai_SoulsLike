// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_HasStamina.generated.h"

UCLASS()
class ALSV4_CPP_API UBTDecorator_HasStamina : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_HasStamina();

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	// The minimum stamina required to allow attacking
	UPROPERTY(EditAnywhere, Category = "Stamina")
	float RequiredStamina = 20.0f;

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
