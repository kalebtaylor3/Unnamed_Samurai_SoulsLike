#include "AI/BTTask_WolfFleeAndShoot.h"

#include "AIController.h"
#include "AI/EnemyWolfCombatComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"

UBTTask_WolfFleeAndShoot::UBTTask_WolfFleeAndShoot()
{
	NodeName = "Wolf Flee And Shoot";
	bCreateNodeInstance = true;
	bNotifyTick = true;
	TargetActorKey.SelectedKeyName = "TargetActor";
}

EBTNodeResult::Type UBTTask_WolfFleeAndShoot::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* AIPawn = AIController ? AIController->GetPawn() : nullptr;
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!AIController || !AIPawn || !Blackboard)
	{
		return EBTNodeResult::Failed;
	}

	ActiveWolfCombat = AIPawn->FindComponentByClass<UEnemyWolfCombatComponent>();
	if (!ActiveWolfCombat || !ActiveWolfCombat->WantsLowHealthFleeAndShoot())
	{
		return EBTNodeResult::Failed;
	}

	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Target)
	{
		return EBTNodeResult::Failed;
	}

	FVector AwayFromTarget = (AIPawn->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal2D();
	if (AwayFromTarget.IsNearlyZero())
	{
		AwayFromTarget = -Target->GetActorForwardVector().GetSafeNormal2D();
	}

	FVector DesiredLocation = Target->GetActorLocation() + AwayFromTarget * ActiveWolfCombat->LowHealthShotClearDistance;
	if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(AIPawn))
	{
		FNavLocation NavLocation;
		if (NavSys->ProjectPointToNavigation(DesiredLocation, NavLocation, FVector(350.0f, 350.0f, 300.0f)))
		{
			DesiredLocation = NavLocation.Location;
		}
	}

	ActiveWolfCombat->BeginLowHealthFlee();
	AIController->MoveToLocation(DesiredLocation, ClearDistanceTolerance);

	ActiveFleeLocation = DesiredLocation;
	ActiveElapsedTime = 0.0f;
	ActiveShootDuration = 0.0f;
	bShotMontageStarted = false;
	Phase = EWolfFleeShootPhase::MovingAway;

	return EBTNodeResult::InProgress;
}

void UBTTask_WolfFleeAndShoot::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* AIPawn = AIController ? AIController->GetPawn() : nullptr;
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!AIController || !AIPawn || !Blackboard || !ActiveWolfCombat)
	{
		FinishFleeShootTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ActiveElapsedTime += DeltaSeconds;
	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Target)
	{
		FinishFleeShootTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	switch (Phase)
	{
	case EWolfFleeShootPhase::MovingAway:
	{
		const float DistanceToTarget = FVector::Dist2D(AIPawn->GetActorLocation(), Target->GetActorLocation());
		const float DistanceToFleePoint = FVector::Dist2D(AIPawn->GetActorLocation(), ActiveFleeLocation);
		const UPathFollowingComponent* PathFollowingComponent = AIController->GetPathFollowingComponent();
		const bool bMoveFinished = PathFollowingComponent && PathFollowingComponent->GetStatus() == EPathFollowingStatus::Idle;
		const bool bClearEnough = DistanceToTarget >= ActiveWolfCombat->LowHealthShotClearDistance - ClearDistanceTolerance;
		const bool bAtFleePoint = DistanceToFleePoint <= FMath::Max(ClearDistanceTolerance, 25.0f);

		if (bClearEnough || bAtFleePoint || bMoveFinished || ActiveElapsedTime >= MaxFleeTravelTime)
		{
			BeginShootFacingPhase(OwnerComp);
		}
		break;
	}
	case EWolfFleeShootPhase::FacingPlayer:
		if (ActiveWolfCombat->IsFacingTargetForLowHealthShot() && ActiveWolfCombat->IsLowHealthShotReady())
		{
			BeginShotMontagePhase();
		}
		break;
	case EWolfFleeShootPhase::Shooting:
		if (ActiveElapsedTime >= ActiveShootDuration + PostShotHoldTime)
		{
			FinishFleeShootTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		break;
	default:
		FinishFleeShootTask(OwnerComp, EBTNodeResult::Failed);
		break;
	}
}

void UBTTask_WolfFleeAndShoot::BeginShootFacingPhase(UBehaviorTreeComponent& OwnerComp)
{
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		AIController->StopMovement();
	}

	if (ActiveWolfCombat)
	{
		ActiveWolfCombat->BeginLowHealthShootFacing();
	}

	Phase = EWolfFleeShootPhase::FacingPlayer;
	ActiveElapsedTime = 0.0f;
}

void UBTTask_WolfFleeAndShoot::BeginShotMontagePhase()
{
	if (!ActiveWolfCombat || bShotMontageStarted)
	{
		return;
	}

	ActiveShootDuration = FMath::Max(ActiveWolfCombat->PlayLowHealthShotMontage(), ActiveWolfCombat->LowHealthShotDelay);
	ActiveElapsedTime = 0.0f;
	bShotMontageStarted = true;
	Phase = EWolfFleeShootPhase::Shooting;
}

void UBTTask_WolfFleeAndShoot::FinishFleeShootTask(UBehaviorTreeComponent& OwnerComp, EBTNodeResult::Type Result)
{
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		AIController->StopMovement();
	}

	CleanupActiveTask();

	FinishLatentTask(OwnerComp, Result);
}

void UBTTask_WolfFleeAndShoot::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		AIController->StopMovement();
	}

	CleanupActiveTask();
}

void UBTTask_WolfFleeAndShoot::CleanupActiveTask()
{
	if (ActiveWolfCombat)
	{
		ActiveWolfCombat->FinishLowHealthShoot();
	}

	ActiveWolfCombat = nullptr;
	ActiveFleeLocation = FVector::ZeroVector;
	ActiveElapsedTime = 0.0f;
	ActiveShootDuration = 0.0f;
	bShotMontageStarted = false;
	Phase = EWolfFleeShootPhase::MovingAway;
}
