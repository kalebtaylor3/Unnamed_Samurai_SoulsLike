#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_WolfCheckIfInRange.generated.h"

UCLASS(Category = ALS, meta = (DisplayName = "Wolf Check If In Range"))
class ALSV4_CPP_API UBTService_WolfCheckIfInRange : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_WolfCheckIfInRange();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsInRangeKey;

	UPROPERTY(EditAnywhere, Category = "Settings")
	float AttackRange = 170.0f;

	UPROPERTY(EditAnywhere, Category = "Settings")
	float AttackRangeExitBuffer = 65.0f;
};
