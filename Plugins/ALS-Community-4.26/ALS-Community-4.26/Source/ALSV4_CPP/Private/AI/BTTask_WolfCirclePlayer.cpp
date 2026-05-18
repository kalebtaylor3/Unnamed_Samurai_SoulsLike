#include "AI/BTTask_WolfCirclePlayer.h"

#include "AIController.h"
#include "AI/EnemyWolfCombatComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_WolfCirclePlayer::UBTTask_WolfCirclePlayer()
{
	NodeName = "Wolf Circle Player";
	bCreateNodeInstance = true;
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_WolfCirclePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* AIPawn = AIController ? AIController->GetPawn() : nullptr;
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!AIController || !AIPawn || !Blackboard)
	{
		return EBTNodeResult::Failed;
	}

	UEnemyWolfCombatComponent* WolfCombat = AIPawn->FindComponentByClass<UEnemyWolfCombatComponent>();
	if (!WolfCombat || !WolfCombat->CanCircleCurrentTarget())
	{
		return EBTNodeResult::Failed;
	}

	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject("TargetActor"));
	if (!Target)
	{
		return EBTNodeResult::Failed;
	}

	const FVector FromTarget = (AIPawn->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal2D();
	const FVector SafeFromTarget = FromTarget.IsNearlyZero() ? -Target->GetActorForwardVector().GetSafeNormal2D() : FromTarget;
	FVector Tangent = FVector::CrossProduct(FVector::UpVector, SafeFromTarget).GetSafeNormal2D();
	if (!bCircleClockwise)
	{
		Tangent *= -1.0f;
	}
	bCircleClockwise = !bCircleClockwise;

	const float MinDistance = FMath::Max(0.0f, MinDesiredDistanceFromTarget);
	const float MaxDistance = FMath::Max(MinDistance, MaxDesiredDistanceFromTarget);
	const float ChosenDistance = MaxDistance > MinDistance
		? FMath::FRandRange(MinDistance, MaxDistance)
		: (MinDistance > 0.0f ? MinDistance : DesiredDistanceFromTarget);

	FVector DesiredLocation = Target->GetActorLocation()
		+ SafeFromTarget * ChosenDistance
		+ Tangent * SideStepDistance;

	if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(AIPawn))
	{
		FNavLocation NavLocation;
		if (NavSys->ProjectPointToNavigation(DesiredLocation, NavLocation, FVector(250.0f, 250.0f, 300.0f)))
		{
			DesiredLocation = NavLocation.Location;
		}
	}

	WolfCombat->BeginCircle();
	AIController->MoveToLocation(DesiredLocation, AcceptanceRadius);

	ActiveWolfCombat = WolfCombat;
	ActiveDesiredLocation = DesiredLocation;
	ActiveTravelTime = 0.0f;
	ActiveHoldTime = 0.0f;
	ActiveHoldDuration = CircleDuration;
	bHoldingAtCirclePoint = false;
	bCircleWarningStarted = false;

	return EBTNodeResult::InProgress;
}

void UBTTask_WolfCirclePlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* AIPawn = AIController ? AIController->GetPawn() : nullptr;
	if (!AIPawn || !ActiveWolfCombat)
	{
		FinishCircleTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (bHoldingAtCirclePoint)
	{
		if (!bCircleWarningStarted)
		{
			if (!ActiveWolfCombat->IsFacingTargetForCircleWarning())
			{
				return;
			}

			ActiveWolfCombat->PlayCircleWarningMontage();
			ActiveHoldDuration = CircleDuration;
			ActiveHoldTime = 0.0f;
			bCircleWarningStarted = true;
		}

		ActiveHoldTime += DeltaSeconds;
		if (ActiveHoldTime >= ActiveHoldDuration)
		{
			FinishCircleTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		return;
	}

	ActiveTravelTime += DeltaSeconds;

	const float DistanceToCirclePoint = FVector::Dist2D(AIPawn->GetActorLocation(), ActiveDesiredLocation);
	const bool bReachedCirclePoint = DistanceToCirclePoint <= FMath::Max(AcceptanceRadius + 75.0f, 75.0f);
	const UPathFollowingComponent* PathFollowingComponent = AIController ? AIController->GetPathFollowingComponent() : nullptr;
	const bool bMoveFinished = PathFollowingComponent && PathFollowingComponent->GetStatus() == EPathFollowingStatus::Idle;

	if (bReachedCirclePoint || bMoveFinished)
	{
		BeginCircleHold(OwnerComp);
		return;
	}

	if (ActiveTravelTime >= MaxTravelTime)
	{
		FinishCircleTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

void UBTTask_WolfCirclePlayer::BeginCircleHold(UBehaviorTreeComponent& OwnerComp)
{
	if (bHoldingAtCirclePoint)
	{
		return;
	}

	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		AIController->StopMovement();
	}

	if (ActiveWolfCombat)
	{
		ActiveWolfCombat->BeginCircleHold();
	}

	bHoldingAtCirclePoint = true;
	bCircleWarningStarted = false;
	ActiveHoldTime = 0.0f;
	ActiveHoldDuration = CircleDuration;
}

void UBTTask_WolfCirclePlayer::FinishCircleTask(UBehaviorTreeComponent& OwnerComp, EBTNodeResult::Type Result)
{
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		AIController->StopMovement();
	}

	if (ActiveWolfCombat)
	{
		ActiveWolfCombat->FinishCircle();
	}

	ActiveWolfCombat = nullptr;
	ActiveDesiredLocation = FVector::ZeroVector;
	ActiveTravelTime = 0.0f;
	ActiveHoldTime = 0.0f;
	ActiveHoldDuration = 0.0f;
	bHoldingAtCirclePoint = false;
	bCircleWarningStarted = false;

	FinishLatentTask(OwnerComp, Result);
}
