#include "AnimNotify_ComboWindow.h"
#include "Character/ALSBaseCharacter.h"
#include "Character/CombatComponent.h"

void UAnimNotify_ComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    if (AALSBaseCharacter* Character = Cast<AALSBaseCharacter>(MeshComp->GetOwner()))
    {
        if (UCombatComponent* Combat = Character->CombatSystem)
        {
            Combat->OnComboWindowOpened();
        }
    }
}

void UAnimNotify_ComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (AALSBaseCharacter* Character = Cast<AALSBaseCharacter>(MeshComp->GetOwner()))
    {
        if (UCombatComponent* Combat = Character->CombatSystem)
        {
            Combat->OnComboWindowClosed();
        }
    }
}