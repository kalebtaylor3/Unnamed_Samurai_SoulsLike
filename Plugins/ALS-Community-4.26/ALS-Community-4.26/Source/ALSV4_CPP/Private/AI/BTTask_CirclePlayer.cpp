// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_CirclePlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "TimerManager.h"

UBTTask_CirclePlayer::UBTTask_CirclePlayer()
{
	NodeName = "Circle Player";
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_CirclePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* AIPawn = AIController->GetPawn();
	if (!AIPawn) return EBTNodeResult::Failed;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject("TargetActor"));
	if (!Target) return EBTNodeResult::Failed;

	const FVector ToTarget = (Target->GetActorLocation() - AIPawn->GetActorLocation()).GetSafeNormal();
	const FVector Up = FVector::UpVector;

	// Alternate direction
	FVector CircleDirection = FVector::CrossProduct(Up, ToTarget).GetSafeNormal();
	if (!bCircleClockwise)
	{
		CircleDirection *= -1.f;
	}

	// Flip for next execution
	bCircleClockwise = !bCircleClockwise;

	FVector DesiredLocation = AIPawn->GetActorLocation() + CircleDirection * CircleRadius;

	// Nav validation
	if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
	{
		FNavLocation NavLoc;
		if (NavSys->GetRandomPointInNavigableRadius(DesiredLocation, 100.f, NavLoc))
		{
			DesiredLocation = NavLoc.Location;
		}
	}

	AIController->MoveToLocation(DesiredLocation, 5.0f);

	FTimerDelegate TimerDel;
	TimerDel.BindLambda([this, &OwnerComp]() {
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		});

	FTimerHandle TempHandle;
	AIPawn->GetWorldTimerManager().SetTimer(TempHandle, TimerDel, CircleDuration, false);

	return EBTNodeResult::InProgress;
}

