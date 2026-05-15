#include "Character/CombatComponent.h"
#include "Animation/AnimInstance.h"
#include "Character/ALSBaseCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Weapons/HeldWeaponBase.h"
#include "Weapons/WeaponArrowProjectile.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::SetCheckingForStanceChange(bool value)
{
	checkingForStanceChange = value;
}

void UCombatComponent::SetStance()
{
	if (!bCanChangeStance)
		return;

	bCanChangeStance = false;
	GetWorld()->GetTimerManager().SetTimer(StanceChangeCooldownTimer, this, &UCombatComponent::EnableStanceChange, 0.5f, false);

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

void UCombatComponent::EnableStanceChange()
{
	bCanChangeStance = true;
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

	if (IsBowEquipped())
	{
		FireBow();
		return;
	}

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

		// Already attacking � queue next if allowed
		if (bIsAttacking)
		{
			// Only queue if input is allowed and we haven�t queued this window yet
			if (bCanReceiveInput && !bInputQueuedThisWindow && AttackIndex + 1 < CurrentMontageList.Num())
			{
				QueuedComboIndices.Enqueue(AttackIndex + 1);
				QueuedCount++;
				bInputQueuedThisWindow = true; // restrict further inputs until next window
			}
			return;
		}

		// Not attacking � start combo
		AttackIndex = 0;
		bIsAttacking = true;
		bCanReceiveInput = false;
		QueuedComboIndices.Empty();
		QueuedCount = 0;
		bInputQueuedThisWindow = false;

		PlayLightAttackMontage(AttackIndex, CurrentMontageList);
	}
}

void UCombatComponent::StartBowDraw()
{
	if (!OwnerCharacter || !CurrentWeapon || !IsBowEquipped())
		return;

	if (checkingForStanceChange || bIsAttacking || bIsDrawingBow)
		return;

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
		return;

	bIsDrawingBow = true;
	bCanFireDrawnBow = false;
	bIsAttacking = true;
	canRoll = false;

	AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);

	if (CurrentWeapon->BowDrawMontage)
	{
		const float DrawDuration = AnimInstance->Montage_Play(CurrentWeapon->BowDrawMontage);
		if (DrawDuration > 0.0f)
		{
			GetWorld()->GetTimerManager().SetTimer(BowDrawReadyTimer, this, &UCombatComponent::FinishBowDraw, DrawDuration, false);
		}
		else
		{
			FinishBowDraw();
		}
	}
	else
	{
		FinishBowDraw();
	}
}

void UCombatComponent::CancelBowDraw()
{
	if (!OwnerCharacter || !CurrentWeapon || !bIsDrawingBow)
		return;

	GetWorld()->GetTimerManager().ClearTimer(BowDrawReadyTimer);

	if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
	{
		if (CurrentWeapon->BowDrawMontage)
		{
			AnimInstance->Montage_Stop(0.15f, CurrentWeapon->BowDrawMontage);
		}

	}

	HideBowPreviewArrow();
	bIsDrawingBow = false;
	bCanFireDrawnBow = false;
	bIsAttacking = false;
	canRoll = true;
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
	if (!OwnerCharacter || !CurrentWeapon)
		return;

	if (OwnerCharacter->GetOverlayState() == EALSOverlayState::Default)
		return;

	if (checkingForStanceChange)
		return;

	if (bIsAttacking || bIsChargingHeavy)
		return;

	if (!CurrentWeapon->ChargeMontage && !CurrentWeapon->ChargeMontageLoop)
		return;

	if (OwnerCharacter->PlayerStats->CurrentStamina < CurrentWeapon->HeavyAttackStaminaAmount)
	{
		OwnerCharacter->PlayerStats->NotifyStaminaExhausted();
		return;
	}

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
		bIsAttacking = true;
		bIsChargingHeavy = true;
		bIsHoldingCharge = true;
		bIsLoopingCharge = false;

		if (CurrentWeapon->ChargeMontage)
		{
			const float ChargeStartDuration = AnimInstance->Montage_Play(CurrentWeapon->ChargeMontage);
			if (CurrentWeapon->ChargeMontageLoop && ChargeStartDuration > 0.f)
			{
				GetWorld()->GetTimerManager().SetTimer(ChargeLoopTimer, this, &UCombatComponent::PlayChargeLoopMontage, ChargeStartDuration, false);
			}
		}
		else
		{
			PlayChargeLoopMontage();
		}
	}
}

void UCombatComponent::ReleaseChargeHeavyAttack()
{
	if (!OwnerCharacter || !CurrentWeapon)
		return;

	if (OwnerCharacter->GetOverlayState() == EALSOverlayState::Default)
		return;

	if (!bIsChargingHeavy)
		return;

	bIsChargingHeavy = false;
	bIsHoldingCharge = false;
	bIsLoopingCharge = false;
	GetWorld()->GetTimerManager().ClearTimer(ChargeLoopTimer);
	GetWorld()->GetTimerManager().ClearTimer(OwnerCharacter->PlayerStats->StaminaRegenHandle);

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		bIsAttacking = false;
		return;
	}

	if (AnimInstance)
	{
		if (CurrentWeapon->ChargeMontage)
		{
			AnimInstance->Montage_Stop(0.f, CurrentWeapon->ChargeMontage);
		}
		if (CurrentWeapon->ChargeMontageLoop)
		{
			AnimInstance->Montage_Stop(0.f, CurrentWeapon->ChargeMontageLoop);
		}

		if (!CurrentWeapon->HeavyAttackMontage)
		{
			bIsAttacking = false;
			return;
		}

		OwnerCharacter->PlayerStats->UseStamina(CurrentWeapon->HeavyAttackStaminaAmount);
		AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);
		AnimInstance->Montage_Play(CurrentWeapon->HeavyAttackMontage);
	}
}

void UCombatComponent::PlayChargeLoopMontage()
{
	if (!OwnerCharacter || !CurrentWeapon || !CurrentWeapon->ChargeMontageLoop)
		return;

	if (!bIsChargingHeavy || !bIsHoldingCharge)
		return;

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
		return;

	if (CurrentWeapon->ChargeMontage)
	{
		AnimInstance->Montage_Stop(0.f, CurrentWeapon->ChargeMontage);
	}

	bIsLoopingCharge = true;
	AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);
	const float LoopDuration = AnimInstance->Montage_Play(CurrentWeapon->ChargeMontageLoop);
	if (LoopDuration > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(ChargeLoopTimer, this, &UCombatComponent::PlayChargeLoopMontage, LoopDuration, false);
	}
}

void UCombatComponent::FinishBowDraw()
{
	if (!OwnerCharacter || !CurrentWeapon || !IsBowEquipped() || !bIsDrawingBow)
	{
		return;
	}

	bCanFireDrawnBow = true;
	ShowBowPreviewArrow();
}

bool UCombatComponent::IsBowEquipped() const
{
	return CurrentWeapon && CurrentWeapon->bIsBow;
}

bool UCombatComponent::FireBow()
{
	if (!OwnerCharacter || !CurrentWeapon || !IsBowEquipped() || !bIsDrawingBow || !bCanFireDrawnBow)
		return false;

	if (!CurrentWeapon->ArrowProjectileClass)
		return false;

	if (OwnerCharacter->PlayerStats->CurrentStamina < CurrentWeapon->LightAttackStaminaAmount)
	{
		OwnerCharacter->PlayerStats->NotifyStaminaExhausted();
		return false;
	}

	const FVector SpawnLocation = GetBowArrowSpawnLocation();
	const FRotator AimRotation = GetBowAimRotation(SpawnLocation);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerCharacter;
	SpawnParams.Instigator = OwnerCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AWeaponArrowProjectile* Arrow = GetWorld()->SpawnActor<AWeaponArrowProjectile>(
		CurrentWeapon->ArrowProjectileClass, SpawnLocation, AimRotation, SpawnParams);

	if (Arrow)
	{
		if (CurrentWeapon->PreviewArrowMesh && Arrow->ArrowMesh && !Arrow->ArrowMesh->GetStaticMesh())
		{
			Arrow->ArrowMesh->SetStaticMesh(CurrentWeapon->PreviewArrowMesh);
		}

		UParticleSystem* ArrowHitEffect = OwnerCharacter->HeldWeaponActor ? OwnerCharacter->HeldWeaponActor->HitEffect : nullptr;
		Arrow->InitializeArrow(CurrentWeapon->ArrowDamage, CurrentWeapon->ArrowSpeed, OwnerCharacter, ArrowHitEffect);
	}

	GetWorld()->GetTimerManager().ClearTimer(BowDrawReadyTimer);
	GetWorld()->GetTimerManager().ClearTimer(OwnerCharacter->PlayerStats->StaminaRegenHandle);
	HideBowPreviewArrow();
	OwnerCharacter->PlayerStats->UseStamina(CurrentWeapon->LightAttackStaminaAmount);

	if (CurrentWeapon->BowFireCameraShake)
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController()))
		{
			PlayerController->ClientStartCameraShake(CurrentWeapon->BowFireCameraShake, CurrentWeapon->BowFireCameraShakeScale);
		}
	}

	if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
	{
		if (CurrentWeapon->BowDrawMontage)
		{
			AnimInstance->Montage_Stop(0.0f, CurrentWeapon->BowDrawMontage);
		}

		if (CurrentWeapon->BowFireMontage)
		{
			AnimInstance->Montage_Play(CurrentWeapon->BowFireMontage);
		}
	}

	bIsDrawingBow = false;
	bCanFireDrawnBow = false;
	bIsAttacking = false;
	canRoll = true;

	return Arrow != nullptr;
}

void UCombatComponent::ShowBowPreviewArrow()
{
	if (OwnerCharacter && OwnerCharacter->HeldWeaponActor)
	{
		OwnerCharacter->HeldWeaponActor->ShowPreviewArrow();
	}
}

void UCombatComponent::HideBowPreviewArrow()
{
	if (OwnerCharacter && OwnerCharacter->HeldWeaponActor)
	{
		OwnerCharacter->HeldWeaponActor->HidePreviewArrow();
	}
}

FVector UCombatComponent::GetBowArrowSpawnLocation() const
{
	if (!OwnerCharacter || !CurrentWeapon)
	{
		return FVector::ZeroVector;
	}

	if (OwnerCharacter->HeldWeaponActor)
	{
		if (UStaticMeshComponent* WeaponMesh = OwnerCharacter->HeldWeaponActor->WeaponMesh)
		{
			if (WeaponMesh->DoesSocketExist(CurrentWeapon->ArrowSpawnSocketName))
			{
				return WeaponMesh->GetSocketLocation(CurrentWeapon->ArrowSpawnSocketName);
			}
		}
	}

	return OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorTransform().TransformVector(CurrentWeapon->ArrowSpawnOffset);
}

FRotator UCombatComponent::GetBowAimRotation(const FVector& SpawnLocation) const
{
	if (!OwnerCharacter || !CurrentWeapon)
	{
		return FRotator::ZeroRotator;
	}

	APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PlayerController)
	{
		return OwnerCharacter->GetActorRotation();
	}

	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FVector TraceStart = ViewLocation;
	const FVector TraceEnd = TraceStart + ViewRotation.Vector() * CurrentWeapon->AimTraceRange;

	FHitResult Hit;
	FCollisionQueryParams QueryParams(FName(TEXT("BowAimTrace")), false, OwnerCharacter);
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	const FVector AimPoint = bHit ? Hit.ImpactPoint : TraceEnd;
	return (AimPoint - SpawnLocation).Rotation();
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
	if (!OwnerCharacter || !CurrentWeapon)
		return;


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
	bIsDrawingBow = false;
	bCanFireDrawnBow = false;
	GetWorld()->GetTimerManager().ClearTimer(BowDrawReadyTimer);
	HideBowPreviewArrow();
	bCanReceiveInput = false;
	bInputQueuedThisWindow = false;
	checkingForStanceChange = false;
	canRoll = true;

	QueuedComboIndices.Empty();
	AttackIndex = 0;

	GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, TEXT("Attack Interrupted!"));
}
