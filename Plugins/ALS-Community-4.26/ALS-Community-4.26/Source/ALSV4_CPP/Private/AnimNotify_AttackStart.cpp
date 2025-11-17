#include "AnimNotify_AttackStart.h"
#include "Character/ALSBaseCharacter.h"
#include "Character/CombatComponent.h"

void UAnimNotify_AttackStart::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (AALSBaseCharacter* Character = Cast<AALSBaseCharacter>(MeshComp->GetOwner()))
    {
        if (UCombatComponent* Combat = Character->CombatSystem)
        {
            Combat->OnAttackStarted();
        }
    }
}