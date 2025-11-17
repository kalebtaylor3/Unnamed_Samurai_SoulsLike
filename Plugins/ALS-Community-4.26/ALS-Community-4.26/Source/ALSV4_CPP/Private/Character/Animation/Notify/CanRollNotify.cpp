// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Animation/Notify/CanRollNotify.h"
#include "Character/ALSBaseCharacter.h"
#include "Character/CombatComponent.h"

void UCanRollNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (AALSBaseCharacter* Character = Cast<AALSBaseCharacter>(MeshComp->GetOwner()))
    {
        if (UCombatComponent* Combat = Character->CombatSystem)
        {
            Combat->canRoll = true;
        }
    }
}