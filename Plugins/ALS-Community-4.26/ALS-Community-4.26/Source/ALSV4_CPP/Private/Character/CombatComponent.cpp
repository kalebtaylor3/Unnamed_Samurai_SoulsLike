#include "Character/CombatComponent.h"
#include "Animation/AnimInstance.h"
#include "Character/ALSBaseCharacter.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsChargingHeavy && bIsHoldingCharge && CurrentWeapon->ChargeMontage)
	{
		UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
		if (!AnimInstance || !AnimInstance->Montage_IsPlaying(CurrentWeapon->ChargeMontage)) return;

		float CurrentPosition = AnimInstance->Montage_GetPosition(CurrentWeapon->ChargeMontage);

		if (!bIsLoopingCharge)
		{
			if (CurrentPosition >= CurrentWeapon->ChargeMontage->GetPlayLength() - 0.05f)
			{
				AnimInstance->Montage_JumpToSection(FName("Loop"), CurrentWeapon->ChargeMontage);
				bIsLoopingCharge = true;
			}
		}
		else
		{
			// Get loop timing directly from the montage (not AnimInstance)
			int32 SectionIndex = CurrentWeapon->ChargeMontage->GetSectionIndex(FName("Loop"));
			float LoopStartTime = 0.f;
			float LoopEndTime = CurrentWeapon->ChargeMontage->GetPlayLength();

			if (SectionIndex != INDEX_NONE)
			{
				LoopStartTime = CurrentWeapon->ChargeMontage->CompositeSections[SectionIndex].GetTime();

				if (SectionIndex + 1 < CurrentWeapon->ChargeMontage->CompositeSections.Num())
				{
					LoopEndTime = CurrentWeapon->ChargeMontage->CompositeSections[SectionIndex + 1].GetTime();
				}
			}

			float LoopDuration = LoopEndTime - LoopStartTime;

			// If at end of loop section, jump again
			if (CurrentPosition >= LoopStartTime + LoopDuration - 0.05f)
			{
				AnimInstance->Montage_JumpToSection(FName("Loop"), CurrentWeapon->ChargeMontage);
			}
		}
	}

}

void UCombatComponent::SetCheckingForStanceChange(bool value)
{
	checkingForStanceChange = value;
}

void UCombatComponent::SetStance()
{
	if (CurrentStance == ECombatStance::OneHanded)
	{
		CurrentStance = ECombatStance::TwoHanded;
	}
	else
	{
		CurrentStance = ECombatStance::OneHanded;
	}

	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Stance Changed"));
	return;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<AALSBaseCharacter>(GetOwner());
}

void UCombatComponent::LightAttack()
{
	if (checkingForStanceChange)
		return;

	if (!CurrentWeapon)
		return;

	if (OwnerCharacter->GetOverlayState() == EALSOverlayState::Default)
		return;

	if (OwnerCharacter->PlayerStats->CurrentStamina < CurrentWeapon->LightAttackStaminaAmount)
	{
		OwnerCharacter->PlayerStats->NotifyStaminaExhausted();
		return;
	}

	if (!OwnerCharacter)
	{
		return;
	}

	if (OwnerCharacter->GetMovementState() == EALSMovementState::InAir)
	{

		if (OwnerCharacter->PlayerStats->CurrentStamina < CurrentWeapon->JumpAttackStaminaAmount)
		{
			OwnerCharacter->PlayerStats->NotifyStaminaExhausted();
			return;
		}

		if (bIsAttacking)
			return;

		UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);

			if (CurrentStance == ECombatStance::OneHanded)
				AnimInstance->Montage_Play(CurrentWeapon->OneHJumpAttackMontage); //AnimInstance->Montage_Play(OneHJumpAttackLightAttackMontage);
			else if (CurrentStance == ECombatStance::TwoHanded)
				AnimInstance->Montage_Play(CurrentWeapon->TwoHJumpAttackMontage);//AnimInstance->Montage_Play(OneHJumpAttackLightAttackMontage);

			GetWorld()->GetTimerManager().ClearTimer(OwnerCharacter->PlayerStats->StaminaRegenHandle);
			OwnerCharacter->PlayerStats->UseStamina(CurrentWeapon->JumpAttackStaminaAmount);
		}
	}
	else if (OwnerCharacter->GetMovementState() == EALSMovementState::Grounded)
	{
		const TArray<UAnimMontage*>& CurrentMontageList =
			(CurrentStance == ECombatStance::TwoHanded) ? CurrentWeapon->TwoHandedLightAttackMontages : CurrentWeapon->OneHandedLightAttackMontages;

		if (CurrentMontageList.Num() == 0)
		{
			return;
		}

		// Already attacking — queue next if allowed
		if (bIsAttacking)
		{
			// Only queue if input is allowed and we haven’t queued this window yet
			if (bCanReceiveInput && !bInputQueuedThisWindow && AttackIndex + 1 < CurrentMontageList.Num())
			{
				QueuedComboIndices.Enqueue(AttackIndex + 1);
				QueuedCount++;
				bInputQueuedThisWindow = true; // restrict further inputs until next window
			}
			return;
		}

		// Not attacking — start combo
		AttackIndex = 0;
		bIsAttacking = true;
		bCanReceiveInput = false;
		QueuedComboIndices.Empty();
		QueuedCount = 0;
		bInputQueuedThisWindow = false;

		PlayLightAttackMontage(AttackIndex, CurrentMontageList);
	}
}

void UCombatComponent::UseAshOfWar()
{
	if (!CurrentWeapon) return;

	if (CurrentWeapon->AshOfWarClass)
	{
		UAshOfWarBase* Ash = NewObject<UAshOfWarBase>(this, CurrentWeapon->AshOfWarClass);
		if (Ash)
		{
			Ash->ActivateAsh(Cast<AALSBaseCharacter>(GetOwner()));
		}
	}
}

void UCombatComponent::PlayLightAttackMontage(int32 Index, const TArray<UAnimMontage*>& MontageList)
{
	if (!MontageList.IsValidIndex(Index)) return;

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		UAnimMontage* MontageToPlay = MontageList[Index];
		if (MontageToPlay)
		{
			// Duplicate montage if first index and set blend time to 0 (this ensures we always attack.. & have control over the direction for a second in time.. )
			if (Index == 0)
			{
				UAnimMontage* TempMontage = DuplicateObject<UAnimMontage>(MontageToPlay, this);
				if (TempMontage)
				{
					TempMontage->BlendIn.SetBlendTime(0.f);
					MontageToPlay = TempMontage;
				}
			}

			AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);
			AnimInstance->Montage_Play(MontageToPlay);
			GetWorld()->GetTimerManager().ClearTimer(OwnerCharacter->PlayerStats->StaminaRegenHandle);
			OwnerCharacter->PlayerStats->UseStamina(CurrentWeapon->LightAttackStaminaAmount);
		}
	}
}

void UCombatComponent::StartChargeHeavyAttack()
{
	if (OwnerCharacter->GetOverlayState() == EALSOverlayState::Default)
		return;

	if (OwnerCharacter->PlayerStats->CurrentStamina < CurrentWeapon->HeavyAttackStaminaAmount)
	{
		OwnerCharacter->PlayerStats->NotifyStaminaExhausted();
		return;
	}

	if (checkingForStanceChange)
		return;

	if (!CurrentWeapon)
		return;

	if (OwnerCharacter->GetOverlayState() == EALSOverlayState::Default || bIsAttacking)
		return;

	if (!OwnerCharacter || !CurrentWeapon->ChargeMontage) return;

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
		bIsAttacking = true;
		bIsChargingHeavy = true;
		bIsHoldingCharge = true;
		bHasJumpedToLoop = false;

		AnimInstance->Montage_Play(CurrentWeapon->ChargeMontage);
	}
}

void UCombatComponent::ReleaseChargeHeavyAttack()
{
	if (OwnerCharacter->GetOverlayState() == EALSOverlayState::Default)
		return;

	if (!CurrentWeapon)
		return;

	if (!OwnerCharacter || !CurrentWeapon->HeavyAttackMontage || !bIsChargingHeavy) return;

	bIsChargingHeavy = false;
	bIsHoldingCharge = false;
	bIsLoopingCharge = false;
	GetWorld()->GetTimerManager().ClearTimer(OwnerCharacter->PlayerStats->StaminaRegenHandle);
	OwnerCharacter->PlayerStats->UseStamina(CurrentWeapon->HeavyAttackStaminaAmount);

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Stop(0.f, CurrentWeapon->ChargeMontage);
		AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);
		AnimInstance->Montage_Play(CurrentWeapon->HeavyAttackMontage);
	}
}

void UCombatComponent::HeavyAttackCheck(bool bValue)
{
	if (bValue)
	{
		StartChargeHeavyAttack();
	}
	else
	{
		ReleaseChargeHeavyAttack();
	}
}


void UCombatComponent::OnAttackStarted()
{
	bIsAttacking = true;
	bCanReceiveInput = false;
	canRoll = false;
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("attack Started"));
}

void UCombatComponent::OnComboWindowOpened()
{
	bCanReceiveInput = true;
	bInputQueuedThisWindow = false; // allow one input during this window
}

void UCombatComponent::OnComboWindowClosed()
{
	bCanReceiveInput = false;
}

void UCombatComponent::OnAttackEnded()
{
	if (!OwnerCharacter || CurrentWeapon->OneHandedLightAttackMontages.Num() == 0)
		return;

	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("attack ended"));

	// If we queued another input, continue the combo
	if (!QueuedComboIndices.IsEmpty())
	{
		const TArray<UAnimMontage*>& CurrentMontageList =
			(CurrentStance == ECombatStance::TwoHanded) ? CurrentWeapon->TwoHandedLightAttackMontages : CurrentWeapon->OneHandedLightAttackMontages;

		int32 NextIndex;
		if (QueuedComboIndices.Dequeue(NextIndex) && CurrentMontageList.IsValidIndex(NextIndex))
		{
			AttackIndex = NextIndex;
			PlayLightAttackMontage(AttackIndex, CurrentMontageList);
			return;
		}
	}

	// Reset state if no valid combo follow-up
	bIsAttacking = false;
	bCanReceiveInput = false;
	AttackIndex = 0;
	QueuedComboIndices.Empty();
}

void UCombatComponent::InterruptAttack()
{
	if (!OwnerCharacter)
		return;

	// Reset all flags
	bIsAttacking = false;
	bIsChargingHeavy = false;
	bIsHoldingCharge = false;
	bIsLoopingCharge = false;
	bCanReceiveInput = false;
	bInputQueuedThisWindow = false;
	checkingForStanceChange = false;
	canRoll = true;

	QueuedComboIndices.Empty();
	AttackIndex = 0;

	GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, TEXT("Attack Interrupted!"));
}
