#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISenseConfig_Sight.h"
#include "WolfAIController.generated.h"

UCLASS()
class ALSV4_CPP_API AWolfAIController : public AAIController
{
	GENERATED_BODY()

public:
	AWolfAIController();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> Behaviour = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAIPerceptionComponent> AIPerception = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UAISenseConfig_Sight> SightConfig = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Blackboard")
	FName TargetActorKeyName = TEXT("TargetActor");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Blackboard")
	FName SightReactKeyName = TEXT("FirstTimeSeeingPlayer");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Blackboard")
	float SightReactResetDelay = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	float LostSightGracePeriod = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	float KeepTargetWhileNearDistance = 700.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	bool bForgetTargetOnLostSight = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception", meta = (EditCondition = "bForgetTargetOnLostSight"))
	float LostSightForgetDistance = 4500.0f;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION(BlueprintCallable, Category = "AI")
	void ClearTargetIfPlayerInvisible();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual FVector GetFocalPointOnActor(const AActor* Actor) const override;

private:
	FTimerHandle SightReactResetTimer;
	FTimerHandle LostSightClearTimer;
	TWeakObjectPtr<AActor> PendingLostSightActor;

	void ResetSightReactFlag();
	void SetCombatTarget(AActor* Actor, bool bTriggerSightReact);
	void ScheduleLostSightClear(AActor* Actor);
	void ClearTargetAfterLostSight();
	void ClearCombatTarget();
	bool ShouldKeepTargetAfterLostSight(const AActor* Actor) const;
};
