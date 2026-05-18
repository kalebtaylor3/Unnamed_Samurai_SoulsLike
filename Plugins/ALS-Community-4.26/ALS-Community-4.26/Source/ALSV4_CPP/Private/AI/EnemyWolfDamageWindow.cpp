#include "AI/EnemyWolfDamageWindow.h"

#include "AI/EnemyWolfCombatComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UEnemyWolfDamageWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	if (!MeshComp)
	{
		return;
	}

	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (UEnemyWolfCombatComponent* WolfCombat = Owner->FindComponentByClass<UEnemyWolfCombatComponent>())
		{
			WolfCombat->BeginBiteDamageWindow(DamageAmount, HitRadius, HitForwardOffset);
		}
	}
}

void UEnemyWolfDamageWindow::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
	if (!MeshComp)
	{
		return;
	}

	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (UEnemyWolfCombatComponent* WolfCombat = Owner->FindComponentByClass<UEnemyWolfCombatComponent>())
		{
			WolfCombat->TickBiteDamageWindow(DamageAmount, HitRadius, HitForwardOffset);
		}
	}
}

void UEnemyWolfDamageWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp)
	{
		return;
	}

	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (UEnemyWolfCombatComponent* WolfCombat = Owner->FindComponentByClass<UEnemyWolfCombatComponent>())
		{
			WolfCombat->EndBiteDamageWindow();
		}
	}
}
