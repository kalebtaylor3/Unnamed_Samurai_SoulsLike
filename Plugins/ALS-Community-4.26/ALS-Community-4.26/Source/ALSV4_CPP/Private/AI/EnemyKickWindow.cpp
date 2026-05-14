// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EnemyKickWindow.h"
#include "AI/EnemyCombatComponent.h"
#include "Character/ALSBaseCharacter.h"

void UEnemyKickWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                   const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AALSBaseCharacter* OwnerCharacter = MeshComp ? Cast<AALSBaseCharacter>(MeshComp->GetOwner()) : nullptr)
	{
		if (UEnemyCombatComponent* CombatComp = OwnerCharacter->FindComponentByClass<UEnemyCombatComponent>())
		{
			CombatComp->BeginKickDamageWindow();
			CombatComp->TickKickDamageWindow(DamageAmount, HitRadius, HitForwardOffset, LaunchStrength, LaunchUpwardStrength);
		}
	}
}

void UEnemyKickWindow::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
                                  const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (AALSBaseCharacter* OwnerCharacter = MeshComp ? Cast<AALSBaseCharacter>(MeshComp->GetOwner()) : nullptr)
	{
		if (UEnemyCombatComponent* CombatComp = OwnerCharacter->FindComponentByClass<UEnemyCombatComponent>())
		{
			CombatComp->TickKickDamageWindow(DamageAmount, HitRadius, HitForwardOffset, LaunchStrength, LaunchUpwardStrength);
		}
	}
}

void UEnemyKickWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                 const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (AALSBaseCharacter* OwnerCharacter = MeshComp ? Cast<AALSBaseCharacter>(MeshComp->GetOwner()) : nullptr)
	{
		if (UEnemyCombatComponent* CombatComp = OwnerCharacter->FindComponentByClass<UEnemyCombatComponent>())
		{
			CombatComp->EndKickDamageWindow();
		}
	}
}
