// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Animation/Notify/AttachWeaponToHandNotify.h"
#include "Character/ALSCharacter.h"
#include "Character/InventoryComponent.h"
#include "Weapons/WeaponBase.h"

void UAttachWeaponToHandNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp)
	{
		return;
	}

	AALSCharacter* Character = Cast<AALSCharacter>(MeshComp->GetOwner());
	if (!Character)
	{
		return;
	}

	FVector AttachLocationOffset = LocationOffset;
	FRotator AttachRotationOffset = RotationOffset;

	if (bUseCurrentWeaponPlacement && Character->Inventory && Character->Inventory->CurrentWeapon)
	{
		AttachLocationOffset = Character->Inventory->CurrentWeapon->PlacementPosition;
		AttachRotationOffset = Character->Inventory->CurrentWeapon->PlacementRotation;
	}

	Character->AttachToHand(nullptr, nullptr, nullptr, bLeftHand, AttachLocationOffset, AttachRotationOffset);
}

FString UAttachWeaponToHandNotify::GetNotifyName_Implementation() const
{
	return TEXT("Attach Weapon To Hand");
}
