#include "AI/WolfAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "Character/ALSBaseCharacter.h"
#include "GameFramework/Pawn.h"
#include "Perception/AISense_Sight.h"

AWolfAIController::AWolfAIController()
{
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	if (SightConfig)
	{
		SightConfig->SightRadius = 2500.0f;
		SightConfig->LoseSightRadius = 3100.0f;
		SightConfig->AutoSuccessRangeFromLastSeenLocation = 700.0f;
		SightConfig->PeripheralVisionAngleDegrees = 75.0f;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	}

	if (AIPerception && SightConfig)
	{
		AIPerception->ConfigureSense(*SightConfig);
		AIPerception->SetDominantSense(UAISense_Sight::StaticClass());
	}
}

void AWolfAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (Behaviour && InPawn)
	{
		RunBehaviorTree(Behaviour);
	}

	if (AIPerception)
	{
		AIPerception->OnTargetPerceptionUpdated.RemoveDynamic(this, &AWolfAIController::OnTargetPerceptionUpdated);
		AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AWolfAIController::OnTargetPerceptionUpdated);
	}
}

FVector AWolfAIController::GetFocalPointOnActor(const AActor* Actor) const
{
	if (!Actor)
	{
		return FAISystem::InvalidLocation;
	}

	if (const APawn* FocusPawn = Cast<APawn>(Actor))
	{
		return FocusPawn->GetPawnViewLocation();
	}

	return Actor->GetActorLocation();
}

void AWolfAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if (!BlackboardComponent || !Actor)
	{
		return;
	}

	if (!Actor->Tags.Contains(FName("Player")))
	{
		return;
	}

	const AALSBaseCharacter* Player = Cast<AALSBaseCharacter>(Actor);
	if (Player && Player->bIsInvisibleToEnemies)
	{
		ClearTargetIfPlayerInvisible();
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		SetCombatTarget(Actor, true);
	}
	else if (bForgetTargetOnLostSight && BlackboardComponent->GetValueAsObject(TargetActorKeyName) == Actor)
	{
		ScheduleLostSightClear(Actor);
	}
}

void AWolfAIController::ClearTargetIfPlayerInvisible()
{
	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if (!BlackboardComponent)
	{
		return;
	}

	AActor* CurrentTarget = Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetActorKeyName));
	const AALSBaseCharacter* Player = Cast<AALSBaseCharacter>(CurrentTarget);
	if (Player && Player->bIsInvisibleToEnemies)
	{
		ClearCombatTarget();
	}
}

void AWolfAIController::ResetSightReactFlag()
{
	if (UBlackboardComponent* BlackboardComponent = GetBlackboardComponent())
	{
		BlackboardComponent->SetValueAsBool(SightReactKeyName, false);
	}
}

void AWolfAIController::SetCombatTarget(AActor* Actor, bool bTriggerSightReact)
{
	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if (!BlackboardComponent || !Actor)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(LostSightClearTimer);
	PendingLostSightActor.Reset();

	BlackboardComponent->SetValueAsObject(TargetActorKeyName, Actor);

	if (bTriggerSightReact)
	{
		BlackboardComponent->SetValueAsBool(SightReactKeyName, true);

		GetWorldTimerManager().ClearTimer(SightReactResetTimer);
		GetWorldTimerManager().SetTimer(
			SightReactResetTimer,
			this,
			&AWolfAIController::ResetSightReactFlag,
			SightReactResetDelay,
			false
		);
	}
}

void AWolfAIController::ScheduleLostSightClear(AActor* Actor)
{
	if (!Actor)
	{
		ClearCombatTarget();
		return;
	}

	PendingLostSightActor = Actor;
	GetWorldTimerManager().ClearTimer(LostSightClearTimer);
	GetWorldTimerManager().SetTimer(
		LostSightClearTimer,
		this,
		&AWolfAIController::ClearTargetAfterLostSight,
		LostSightGracePeriod,
		false
	);
}

void AWolfAIController::ClearTargetAfterLostSight()
{
	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	AActor* PendingActor = PendingLostSightActor.Get();
	if (!BlackboardComponent || !PendingActor)
	{
		ClearCombatTarget();
		return;
	}

	if (BlackboardComponent->GetValueAsObject(TargetActorKeyName) != PendingActor)
	{
		PendingLostSightActor.Reset();
		return;
	}

	if (ShouldKeepTargetAfterLostSight(PendingActor))
	{
		ScheduleLostSightClear(PendingActor);
		return;
	}

	const APawn* ControlledPawn = GetPawn();
	if (ControlledPawn && FVector::Dist2D(ControlledPawn->GetActorLocation(), PendingActor->GetActorLocation()) <= LostSightForgetDistance)
	{
		ScheduleLostSightClear(PendingActor);
		return;
	}

	ClearCombatTarget();
}

void AWolfAIController::ClearCombatTarget()
{
	if (UBlackboardComponent* BlackboardComponent = GetBlackboardComponent())
	{
		BlackboardComponent->ClearValue(TargetActorKeyName);
		BlackboardComponent->SetValueAsBool(SightReactKeyName, false);
		BlackboardComponent->SetValueAsBool("ShouldCircle", false);
	}

	GetWorldTimerManager().ClearTimer(LostSightClearTimer);
	PendingLostSightActor.Reset();
	ClearFocus(EAIFocusPriority::Gameplay);
}

bool AWolfAIController::ShouldKeepTargetAfterLostSight(const AActor* Actor) const
{
	const APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || !Actor)
	{
		return false;
	}

	return FVector::Dist2D(ControlledPawn->GetActorLocation(), Actor->GetActorLocation()) <= KeepTargetWhileNearDistance;
}
