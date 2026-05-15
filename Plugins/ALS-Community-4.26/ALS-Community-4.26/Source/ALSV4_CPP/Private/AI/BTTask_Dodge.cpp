// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_Dodge.h"
#include "AIController.h"
#include "Character/ALSBaseCharacter.h"
#include "AI/EnemyCombatComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"

namespace
{
	void ClearWasHit(UBehaviorTreeComponent& OwnerComp)
	{
		if (UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent())
		{
			Blackboard->SetValueAsBool("WasHit", false);
		}
	}
}

UBTTask_Dodge::UBTTask_Dodge()
{
	NodeName = "Dodge";
	bNotifyTick = true;
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_Dodge::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		if (AALSBaseCharacter* Enemy = Cast<AALSBaseCharacter>(AIController->GetPawn()))
		{
			CombatComponent = Enemy->FindComponentByClass<UEnemyCombatComponent>();

			if (CombatComponent && CombatComponent->HasDodgeMontage())
			{
				CombatComponent->FaceTargetOnce();

				if (!ShouldAttemptDodge(OwnerComp, Enemy))
				{
					ClearWasHit(OwnerComp);
					return bSucceedWhenSkippingDodge ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
				}

				if (!CombatComponent->TryPlayDodgeMontage())
				{
					ClearWasHit(OwnerComp);
					return EBTNodeResult::Failed;
				}

				UAnimMontage* ActiveDodgeMontage = CombatComponent->GetActiveDodgeMontage();
				if (!ActiveDodgeMontage)
				{
					ClearWasHit(OwnerComp);
					return EBTNodeResult::Failed;
				}

				DodgeDuration = ActiveDodgeMontage->GetPlayLength();
				ElapsedTime = 0.f;
				bIsWaitingForMontageEnd = true;
				LastDodgeTime = Enemy->GetWorld()->GetTimeSeconds();

				return EBTNodeResult::InProgress;
			}
		}
	}

	ClearWasHit(OwnerComp);
	return EBTNodeResult::Failed;
}

void UBTTask_Dodge::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!bIsWaitingForMontageEnd || !CombatComponent)
	{
		ClearWasHit(OwnerComp);
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ElapsedTime += DeltaSeconds;

	if (ElapsedTime >= DodgeDuration)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);

		if (OwnerComp.GetBlackboardComponent())
		{
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("ShouldDodge", false);
		}

		ClearWasHit(OwnerComp);
	}
}

bool UBTTask_Dodge::ShouldAttemptDodge(UBehaviorTreeComponent& OwnerComp, AALSBaseCharacter* Enemy) const
{
	if (!Enemy)
	{
		return false;
	}

	const UWorld* World = Enemy->GetWorld();
	if (!World || World->GetTimeSeconds() - LastDodgeTime < MinimumTimeBetweenDodges)
	{
		return false;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		return false;
	}

	if (Blackboard->GetValueAsBool("ShouldDodge"))
	{
		return true;
	}

	if (bRequireShouldDodge || !bAllowProximityDodge)
	{
		return false;
	}

	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject("TargetActor"));
	if (!Target)
	{
		return false;
	}

	const float DistanceToTarget = FVector::Dist2D(Enemy->GetActorLocation(), Target->GetActorLocation());
	if (DistanceToTarget > ProximityDodgeRange)
	{
		return false;
	}

	return FMath::FRand() <= ProximityDodgeChance;
}
