// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Animation/Notify/AnimNotifyState_DamageWindow.h"
#include "Weapons/HeldWeaponBase.h"

void UAnimNotifyState_DamageWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	if (AALSBaseCharacter* OwnerCharacter = Cast<AALSBaseCharacter>(MeshComp->GetOwner()))
	{
		if (AHeldWeaponBase* HeldWeapon = OwnerCharacter->HeldWeaponActor)
		{
			HeldWeapon->EnableDamageCollision(DamageAmount);
		}
	}
}

void UAnimNotifyState_DamageWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (AALSBaseCharacter* OwnerCharacter = Cast<AALSBaseCharacter>(MeshComp->GetOwner()))
	{
		if (AHeldWeaponBase* HeldWeapon = OwnerCharacter->HeldWeaponActor)
		{
			HeldWeapon->DisableDamageCollision();
		}
	}
}
