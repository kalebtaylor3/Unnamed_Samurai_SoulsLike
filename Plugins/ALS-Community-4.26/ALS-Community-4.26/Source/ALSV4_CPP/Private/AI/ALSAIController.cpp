// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community

#include "AI/ALSAIController.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Character/ALSBaseCharacter.h"

AALSAIController::AALSAIController()
{
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
}

void AALSAIController::BeginPlay()
{
	Super::BeginPlay(); // Don't forget this

	// 🔥 Hook up the perception update event
}

void AALSAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (Behaviour && InPawn)
	{
		RunBehaviorTree(Behaviour);
		Blackboard = GetBlackboardComponent();
		AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AALSAIController::OnTargetPerceptionUpdated);
	}
}

FVector AALSAIController::GetFocalPointOnActor(const AActor* Actor) const
{
	if (Actor == nullptr)
	{
		return FAISystem::InvalidLocation;
	}
	const APawn* FocusPawn = Cast<APawn>(Actor);
	if (FocusPawn)
	{
		// Focus on pawn's eye view point
		return FocusPawn->GetPawnViewLocation();
	}
	return Actor->GetActorLocation();
}

void AALSAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Blackboard || !Actor) return;

	// ✅ Only react if actor has "Player" tag
	if (!Actor->Tags.Contains(FName("Player"))) return;

	AALSBaseCharacter* Player = Cast<AALSBaseCharacter>(Actor);
	if (Player && Player->bIsInvisibleToEnemies)
		return;

	if (Stimulus.WasSuccessfullySensed())
	{
		Blackboard->SetValueAsObject("TargetActor", Actor);
		Blackboard->SetValueAsBool("FirstTimeSeeingPlayer", true);

		// ⏳ Reset flag after 2 seconds
		GetWorldTimerManager().ClearTimer(ResetFirstTimeSeeingPlayerHandle);
		GetWorldTimerManager().SetTimer(ResetFirstTimeSeeingPlayerHandle, this, &AALSAIController::ResetFirstTimeSeeingPlayer, 2.0f, false);
	}
	else
	{
		Blackboard->ClearValue("TargetActor");
		Blackboard->GetBrainComponent()->GetAIOwner()->SetFocus(nullptr);
	}
}

void AALSAIController::ResetFirstTimeSeeingPlayer()
{
	if (Blackboard)
	{
		Blackboard->SetValueAsBool("FirstTimeSeeingPlayer", false);
	}
}

void AALSAIController::ClearTargetIfPlayerInvisible()
{
	if (!Blackboard) return;

	AActor* CurrentTarget = Cast<AActor>(Blackboard->GetValueAsObject("TargetActor"));
	AALSBaseCharacter* Player = Cast<AALSBaseCharacter>(CurrentTarget);

	if (Player && Player->bIsInvisibleToEnemies)
	{
		Blackboard->ClearValue("TargetActor");
		Blackboard->SetValueAsBool("FirstTimeSeeingPlayer", false);
		Blackboard->GetBrainComponent()->GetAIOwner()->SetFocus(nullptr);
	}
}

