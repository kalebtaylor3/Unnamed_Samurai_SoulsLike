// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community


#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ALSAIController.generated.h"

/**
 * 
 */
UCLASS()
class ALSV4_CPP_API AALSAIController : public AAIController
{
	GENERATED_BODY()

public:
	AALSAIController();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> Behaviour = nullptr;

	// AALSAIController.h
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAIPerceptionComponent* AIPerception;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void ClearTargetIfPlayerInvisible();

protected:
	virtual void OnPossess(APawn* InPawn) override;

	virtual FVector GetFocalPointOnActor(const AActor *Actor) const override;

	virtual void BeginPlay() override;

	FTimerHandle ResetFirstTimeSeeingPlayerHandle;

	UFUNCTION()
	void ResetFirstTimeSeeingPlayer();
};
