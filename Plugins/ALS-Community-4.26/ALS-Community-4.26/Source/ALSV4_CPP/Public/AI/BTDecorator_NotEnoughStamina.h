// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_NotEnoughStamina.generated.h"

UCLASS()
class ALSV4_CPP_API UBTDecorator_NotEnoughStamina : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_NotEnoughStamina();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	UPROPERTY(EditAnywhere, Category = "Stamina")
	float RequiredStamina = 10.f; // Fallback threshold if needed
};
