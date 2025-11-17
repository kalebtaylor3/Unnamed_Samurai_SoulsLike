// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EnemyDamageWindow.h"
#include "AI/EnemyHeldWeaponBase.h"

void UEnemyDamageWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	if (AALSBaseCharacter* OwnerCharacter = Cast<AALSBaseCharacter>(MeshComp->GetOwner()))
	{
		if (UEnemyCombatComponent* CombatComp = OwnerCharacter->FindComponentByClass<UEnemyCombatComponent>())
		{
			if (AEnemyHeldWeaponBase* HeldWeapon = CombatComp->HeldWeaponActor)
			{
				HeldWeapon->EnableDamageCollision(DamageAmount);
			}
		}
	}
}

void UEnemyDamageWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (AALSBaseCharacter* OwnerCharacter = Cast<AALSBaseCharacter>(MeshComp->GetOwner()))
	{
		if (UEnemyCombatComponent* CombatComp = OwnerCharacter->FindComponentByClass<UEnemyCombatComponent>())
		{
			if (AEnemyHeldWeaponBase* HeldWeapon = CombatComp->HeldWeaponActor)
			{
				HeldWeapon->DisableDamageCollision();
			}
		}
	}
}
