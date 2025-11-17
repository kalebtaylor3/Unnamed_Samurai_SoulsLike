// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Animation/Notify/HealCompleteNotify.h"
#include "Character/ALSBaseCharacter.h"


void UHealCompleteNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (AALSBaseCharacter* Character = Cast<AALSBaseCharacter>(MeshComp->GetOwner()))
    {
        Character->Inventory->bHealing = false;
    }
}