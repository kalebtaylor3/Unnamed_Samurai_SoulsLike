#include "AI/EnemyCombatComponent.h"
#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Character/ALSBaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "AI/EnemyHealthComponent.h"
#include "AI/EnemyHeldWeaponBase.h"
#include "Components/BoxComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Library/ALSCharacterEnumLibrary.h"
#include "Character/PlayerStatsComponent.h"


UEnemyCombatComponent::UEnemyCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEnemyCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<AALSBaseCharacter>(GetOwner());
	TargetActorKey.SelectedKeyName = "TargetActor";
	SpawnLocation = GetOwner()->GetActorLocation();

	if (AAIController* AIController = Cast<AAIController>(OwnerCharacter->GetController()))
	{
		Blackboard = AIController->GetBlackboardComponent();
	}

	if (UChildActorComponent* WeaponChildActorComp = OwnerCharacter->FindComponentByClass<UChildActorComponent>())
	{
		// Try cast to AEnemyHeldWeaponBase
		AEnemyHeldWeaponBase* FoundWeapon = Cast<AEnemyHeldWeaponBase>(WeaponChildActorComp->GetChildActor());
		if (FoundWeapon)
		{
			HeldWeaponActor = FoundWeapon;

			if (HeldWeaponActor->DamageHitbox && GetOwner())
			{
				HeldWeaponActor->DamageHitbox->OnComponentBeginOverlap.AddDynamic(
					HeldWeaponActor, &AEnemyHeldWeaponBase::OnDamageHitboxOverlap);
				HeldWeaponActor->DamageHitbox->IgnoreActorWhenMoving(GetOwner(), true);
				GEngine->AddOnScreenDebugMessage((int32)((uintptr_t)this), 0.f, FColor::Green,
					TEXT("bound event from combat component"));
			}
			UE_LOG(LogTemp, Warning, TEXT("Enemy weapon assigned successfully!"));
		}
	}
}

void UEnemyCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (OwnerCharacter)
	{
		if (UEnemyHealthComponent* HealthComponent = OwnerCharacter->FindComponentByClass<UEnemyHealthComponent>())
		{
			if (HealthComponent->IsDeadOrOutOfHealth())
			{
				return;
			}
		}
	}

	AActor* CurrentTarget = Blackboard ? Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;

	switch (CurrentState)
	{
	case EEnemyAIState::Idle:
		TickIdle();
		break;
	case EEnemyAIState::Chasing:
		TickChasing();
		break;
	case EEnemyAIState::Attacking:
		TickAttacking();
		break;
	}

	if (bShouldRotateToTarget && OwnerCharacter)
	{
		if (RotationInterpRemainingTime > 0.f)
		{
			const float Alpha = 1.f - (RotationInterpRemainingTime / RotationInterpDuration);
			const FRotator NewRot = FMath::Lerp(RotationStart, RotationTarget, Alpha);
			OwnerCharacter->SetActorRotation(NewRot);
			RotationInterpRemainingTime -= DeltaTime;
		}
		else
		{
			OwnerCharacter->SetActorRotation(RotationTarget);
			bShouldRotateToTarget = false;
		}
	}

	TickManualLateralDodgeMovement(DeltaTime);
	TickKickKnockback(DeltaTime);
}

void UEnemyCombatComponent::EnterState(EEnemyAIState NewState)
{
	if (CurrentState != NewState)
	{
		CurrentState = NewState;

		if (OwnerCharacter)
		{
			const EALSGait DesiredCombatGait = CurrentState == EEnemyAIState::Chasing
				? EALSGait::Sprinting
				: EALSGait::Running;

			OwnerCharacter->SetDesiredGait(DesiredCombatGait);
		}
	}
}

void UEnemyCombatComponent::TickIdle()
{
	if (!bIsAttacking)
	{
		AActor* Target = Blackboard ? Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;
		if (Target)
		{
			EnterState(EEnemyAIState::Chasing);
		}
	}
}

void UEnemyCombatComponent::TickChasing()
{
	return;
}

void UEnemyCombatComponent::TickAttacking()
{
	return;
}

void UEnemyCombatComponent::PerformAttack()
{
	if (OwnerCharacter)
	{
		if (UEnemyHealthComponent* HealthComponent = OwnerCharacter->FindComponentByClass<UEnemyHealthComponent>())
		{
			if (HealthComponent->IsDeadOrOutOfHealth())
			{
				return;
			}
		}
	}

	if (!OwnerCharacter || bIsAttacking || AttackMontages.Num() == 0 || CurrentStamina <= 0.f)
		return;

	if (!CanAttackCurrentTarget())
	{
		bComboOngoing = false;
		ComboIndex = 0;
		EnterState(EEnemyAIState::Chasing);

		if (Blackboard)
		{
			Blackboard->SetValueAsBool("IsInAttackRange", false);
		}

		return;
	}

	FaceTargetOnce();

	if (!bComboOngoing)
	{
		// ?? Choose a random start index
		ComboStartIndex = FMath::RandRange(0, AttackMontages.Num() - 1);
		ComboIndex = 0;
	}

	const int32 MontageIndex = (ComboStartIndex + ComboIndex) % AttackMontages.Num();

	if (MontageIndex >= AttackMontages.Num())
		return;

	const float RequiredStamina = ComboStaminaCosts.IsValidIndex(MontageIndex)
		? ComboStaminaCosts[MontageIndex] : 20.0f;

	if (CurrentStamina < RequiredStamina)
	{
		ComboIndex = 0;
		return;
	}

	bIsAttacking = true;
	bComboOngoing = true;
	EnterState(EEnemyAIState::Attacking);

	UAnimMontage* SelectedMontage = AttackMontages[MontageIndex];
	CurrentStamina -= RequiredStamina;

	if (SelectedMontage)
	{
		if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
		{
			GetWorld()->GetTimerManager().ClearTimer(StaminaRegenTimer);
			AnimInstance->Montage_Play(SelectedMontage);
			const float Duration = SelectedMontage->GetPlayLength();
			GetWorld()->GetTimerManager().SetTimer(CooldownTimer, this, &UEnemyCombatComponent::ContinueCombo, Duration, false);
		}

		OwnerCharacter->GetCharacterMovement()->StopMovementImmediately();
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_None);
	}

	ComboIndex++;
}

bool UEnemyCombatComponent::CanAttackCurrentTarget() const
{
	const AActor* Target = Blackboard ? Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;
	return CanAttackTarget(Target);
}

bool UEnemyCombatComponent::CanAttackTarget(const AActor* Target) const
{
	if (!OwnerCharacter || !Target)
	{
		return false;
	}

	return FVector::Dist2D(Target->GetActorLocation(), OwnerCharacter->GetActorLocation()) <= AttackRange;
}

bool UEnemyCombatComponent::CanKickCurrentTarget() const
{
	const AActor* Target = Blackboard ? Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;
	if (!OwnerCharacter || !Target || !KickMontage)
	{
		return false;
	}

	return FVector::Dist2D(Target->GetActorLocation(), OwnerCharacter->GetActorLocation()) <= KickRange;
}

void UEnemyCombatComponent::FaceTargetOnce()
{
	if (!OwnerCharacter || !Blackboard) return;

	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Target) return;

	const FVector ToTarget = (Target->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal2D();
	FRotator DesiredRotation = ToTarget.Rotation();

	RotationStart = OwnerCharacter->GetActorRotation();
	RotationTarget = FRotator(0.f, DesiredRotation.Yaw, 0.f);
	RotationInterpRemainingTime = RotationInterpDuration;
	bShouldRotateToTarget = true;
}

void UEnemyCombatComponent::ContinueCombo()
{
	bIsAttacking = false;

	OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	AActor* Target = Blackboard ? Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;

	const bool bInRange = CanAttackTarget(Target);

	const float NextStaminaCost = ComboStaminaCosts.IsValidIndex(ComboIndex + 1) ? ComboStaminaCosts[ComboIndex + 1] : 999.f;

	if (Target && ShouldDodgeBetweenAttacks(Target))
	{
		bComboOngoing = false;
		ComboIndex = 0;
		LastBetweenAttackDodgeTime = GetWorld()->GetTimeSeconds();

		if (ShouldKickBetweenAttacks(Target) && TryPlayKickMontage())
		{
			return;
		}

		RequestDodge();
		return;
	}

	if (bInRange && CurrentStamina >= NextStaminaCost && ComboIndex + 1 < AttackMontages.Num())
	{
		ComboIndex++;
		PerformAttack(); // Continue combo
	}
	else
	{
		bComboOngoing = false;
		ComboIndex = 0;

		// Start stamina regen now that combo is over
		StartStaminaRegen();

		if (Target)
		{
			const float Distance = FVector::Dist2D(Target->GetActorLocation(), OwnerCharacter->GetActorLocation());

			if (Distance <= DodgeTriggerRange && HasDodgeMontage())
			{
				RequestDodge();

				return; // Let behavior tree handle the dodge
			}

			EnterState(EEnemyAIState::Chasing);
		}
		else
		{
			EnterState(EEnemyAIState::Idle);
		}
	}
}

bool UEnemyCombatComponent::ShouldDodgeBetweenAttacks(const AActor* Target) const
{
	if (!OwnerCharacter || !Target || !HasDodgeMontage() || !GetWorld())
	{
		return false;
	}

	if (GetWorld()->GetTimeSeconds() - LastBetweenAttackDodgeTime < MinTimeBetweenAttackDodges)
	{
		return false;
	}

	const float Distance = FVector::Dist2D(Target->GetActorLocation(), OwnerCharacter->GetActorLocation());
	if (Distance > BetweenAttackDodgeRange)
	{
		return false;
	}

	return FMath::FRand() <= BetweenAttackDodgeChance;
}

bool UEnemyCombatComponent::ShouldKickBetweenAttacks(const AActor* Target) const
{
	if (!OwnerCharacter || !Target || !KickMontage)
	{
		return false;
	}

	const float Distance = FVector::Dist2D(Target->GetActorLocation(), OwnerCharacter->GetActorLocation());
	if (Distance > KickRange)
	{
		return false;
	}

	return FMath::FRand() <= BetweenAttackKickChance;
}

void UEnemyCombatComponent::RequestDodge()
{
	StartStaminaRegen();

	if (Blackboard)
	{
		Blackboard->SetValueAsBool("ShouldDodge", true);
	}
}

void UEnemyCombatComponent::StartStaminaRegen()
{
	if (!GetWorld() || CurrentStamina >= MaxStamina)
	{
		return;
	}

	if (!GetWorld()->GetTimerManager().IsTimerActive(StaminaRegenTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(StaminaRegenTimer, this, &UEnemyCombatComponent::RegenStamina, 0.25f, true);
	}
}

void UEnemyCombatComponent::RegenStamina()
{
	if (bIsAttacking || bComboOngoing)
		return;

	CurrentStamina = FMath::Clamp(CurrentStamina + (StaminaRegenRate * 0.25f), 0.f, MaxStamina);

	if (CurrentStamina >= MaxStamina)
	{
		GetWorld()->GetTimerManager().ClearTimer(StaminaRegenTimer);
	}
}

void UEnemyCombatComponent::OnAttackFinished()
{
	if (!bIsAttacking) return; // prevent double call
	bIsAttacking = false;

	OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	AActor* Target = Blackboard ? Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;
	if (Target)
	{
		EnterState(EEnemyAIState::Chasing);
	}
	else
	{
		EnterState(EEnemyAIState::Idle);
	}
}

void UEnemyCombatComponent::PlayDodgeMontage()
{
	TryPlayDodgeMontage();
}

bool UEnemyCombatComponent::TryPlayDodgeMontage()
{
	return TryPlayDodgeMontage(ChooseDodgeDirection());
}

bool UEnemyCombatComponent::TryPlayDodgeMontage(EEnemyDodgeDirection DodgeDirection)
{
	if (!HasDodgeMontage() || !OwnerCharacter || bIsAttacking)
	{
		return false;
	}

	UAnimMontage* SelectedDodgeMontage = GetDodgeMontageForDirection(DodgeDirection);
	if (!SelectedDodgeMontage)
	{
		return false;
	}

	if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
	{

		ActiveDodgeMontage = SelectedDodgeMontage;
		AnimInstance->Montage_Play(SelectedDodgeMontage);
		bIsAttacking = true;

		OwnerCharacter->GetCharacterMovement()->StopMovementImmediately();
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Flying);

		// Reset after dodge montage duration
		const float Duration = SelectedDodgeMontage->GetPlayLength();
		StartManualLateralDodgeMovement(DodgeDirection, Duration);
		GetWorld()->GetTimerManager().SetTimer(CooldownTimer, this, &UEnemyCombatComponent::OnDodgeFinished, Duration, false);

		GetWorld()->GetTimerManager().ClearTimer(DodgeInvincibilityDelayTimer);
		if (DodgeInvincibilityDelay > 0.0f)
		{
			GetWorld()->GetTimerManager().SetTimer(
				DodgeInvincibilityDelayTimer,
				this,
				&UEnemyCombatComponent::BeginDodgeInvincibility,
				DodgeInvincibilityDelay,
				false
			);
		}
		else
		{
			BeginDodgeInvincibility();
		}

		return true;
	}

	return false;
}

void UEnemyCombatComponent::BeginDodgeInvincibility()
{
	if (!OwnerCharacter || DodgeInvincibilityDuration <= 0.0f)
	{
		return;
	}

	if (UEnemyHealthComponent* HealthComponent = OwnerCharacter->FindComponentByClass<UEnemyHealthComponent>())
	{
		HealthComponent->SetInvincibleForDuration(DodgeInvincibilityDuration);
	}
}

bool UEnemyCombatComponent::HasDodgeMontage() const
{
	return DodgeMontage || LeftDodgeMontage || RightDodgeMontage;
}

EEnemyDodgeDirection UEnemyCombatComponent::ChooseDodgeDirection() const
{
	const float AvailableBackwardWeight = DodgeMontage ? FMath::Max(BackwardDodgeWeight, 0.0f) : 0.0f;
	const float AvailableLeftWeight = LeftDodgeMontage ? FMath::Max(LeftDodgeWeight, 0.0f) : 0.0f;
	const float AvailableRightWeight = RightDodgeMontage ? FMath::Max(RightDodgeWeight, 0.0f) : 0.0f;

	const float TotalWeight = AvailableBackwardWeight + AvailableLeftWeight + AvailableRightWeight;
	if (TotalWeight <= 0.0f)
	{
		if (LeftDodgeMontage)
		{
			return EEnemyDodgeDirection::Left;
		}

		if (RightDodgeMontage)
		{
			return EEnemyDodgeDirection::Right;
		}

		return EEnemyDodgeDirection::Backward;
	}

	const float Roll = FMath::FRandRange(0.0f, TotalWeight);
	if (Roll < AvailableBackwardWeight)
	{
		return EEnemyDodgeDirection::Backward;
	}

	if (Roll < AvailableBackwardWeight + AvailableLeftWeight)
	{
		return EEnemyDodgeDirection::Left;
	}

	return EEnemyDodgeDirection::Right;
}

UAnimMontage* UEnemyCombatComponent::GetDodgeMontageForDirection(EEnemyDodgeDirection DodgeDirection) const
{
	switch (DodgeDirection)
	{
	case EEnemyDodgeDirection::Left:
		return LeftDodgeMontage ? LeftDodgeMontage : DodgeMontage;
	case EEnemyDodgeDirection::Right:
		return RightDodgeMontage ? RightDodgeMontage : DodgeMontage;
	case EEnemyDodgeDirection::Backward:
	default:
		return DodgeMontage;
	}
}

void UEnemyCombatComponent::StartManualLateralDodgeMovement(EEnemyDodgeDirection DodgeDirection, float DodgeDuration)
{
	StopManualLateralDodgeMovement();

	if (!bUseManualLateralDodgeMovement || !OwnerCharacter || ManualLateralDodgeDistance <= 0.0f || DodgeDuration <= 0.0f)
	{
		return;
	}

	if (DodgeDirection == EEnemyDodgeDirection::Backward)
	{
		return;
	}

	const FVector RightVector = OwnerCharacter->GetActorRightVector().GetSafeNormal2D();
	if (RightVector.IsNearlyZero())
	{
		return;
	}

	ManualLateralDodgeDirection = DodgeDirection == EEnemyDodgeDirection::Left ? -RightVector : RightVector;
	ManualLateralDodgeDuration = DodgeDuration;
	ManualLateralDodgeElapsedTime = 0.0f;
	ManualLateralDodgePreviousAlpha = 0.0f;
	bIsManualLateralDodgeActive = true;
}

void UEnemyCombatComponent::TickManualLateralDodgeMovement(float DeltaTime)
{
	if (!bIsManualLateralDodgeActive || !OwnerCharacter)
	{
		return;
	}

	ManualLateralDodgeElapsedTime = FMath::Min(ManualLateralDodgeElapsedTime + DeltaTime, ManualLateralDodgeDuration);

	const float RawAlpha = ManualLateralDodgeDuration > 0.0f
		? ManualLateralDodgeElapsedTime / ManualLateralDodgeDuration
		: 1.0f;
	const float CurrentAlpha = FMath::InterpEaseOut(0.0f, 1.0f, RawAlpha, 2.0f);
	const float DeltaDistance = (CurrentAlpha - ManualLateralDodgePreviousAlpha) * ManualLateralDodgeDistance;

	if (!FMath::IsNearlyZero(DeltaDistance))
	{
		FHitResult Hit;
		OwnerCharacter->AddActorWorldOffset(ManualLateralDodgeDirection * DeltaDistance, true, &Hit);

		if (Hit.bBlockingHit)
		{
			StopManualLateralDodgeMovement();
			return;
		}
	}

	ManualLateralDodgePreviousAlpha = CurrentAlpha;

	if (ManualLateralDodgeElapsedTime >= ManualLateralDodgeDuration)
	{
		StopManualLateralDodgeMovement();
	}
}

void UEnemyCombatComponent::StopManualLateralDodgeMovement()
{
	bIsManualLateralDodgeActive = false;
	ManualLateralDodgeDirection = FVector::ZeroVector;
	ManualLateralDodgeElapsedTime = 0.0f;
	ManualLateralDodgeDuration = 0.0f;
	ManualLateralDodgePreviousAlpha = 0.0f;
}

void UEnemyCombatComponent::StartKickKnockback(ACharacter* HitCharacter, const FVector& Direction, float LaunchStrength)
{
	if (!HitCharacter || LaunchStrength <= 0.0f)
	{
		return;
	}

	StopKickKnockback();

	KickKnockbackTarget = HitCharacter;
	KickKnockbackDirection = Direction.GetSafeNormal2D();
	KickKnockbackDistance = LaunchStrength * 0.4f;
	KickKnockbackElapsedTime = 0.0f;
	KickKnockbackPreviousAlpha = 0.0f;
}

void UEnemyCombatComponent::TickKickKnockback(float DeltaTime)
{
	if (!KickKnockbackTarget || KickKnockbackDirection.IsNearlyZero())
	{
		return;
	}

	KickKnockbackElapsedTime = FMath::Min(KickKnockbackElapsedTime + DeltaTime, KickKnockbackDuration);

	const float RawAlpha = KickKnockbackDuration > 0.0f
		? KickKnockbackElapsedTime / KickKnockbackDuration
		: 1.0f;
	const float CurrentAlpha = FMath::InterpEaseOut(0.0f, 1.0f, RawAlpha, 2.0f);
	const float DeltaDistance = (CurrentAlpha - KickKnockbackPreviousAlpha) * KickKnockbackDistance;

	if (!FMath::IsNearlyZero(DeltaDistance))
	{
		FHitResult Hit;
		KickKnockbackTarget->AddActorWorldOffset(KickKnockbackDirection * DeltaDistance, true, &Hit);

		if (Hit.bBlockingHit)
		{
			StopKickKnockback();
			return;
		}
	}

	KickKnockbackPreviousAlpha = CurrentAlpha;

	if (KickKnockbackElapsedTime >= KickKnockbackDuration)
	{
		StopKickKnockback();
	}
}

void UEnemyCombatComponent::StopKickKnockback()
{
	KickKnockbackTarget = nullptr;
	KickKnockbackDirection = FVector::ZeroVector;
	KickKnockbackDistance = 0.0f;
	KickKnockbackElapsedTime = 0.0f;
	KickKnockbackPreviousAlpha = 0.0f;
}

void UEnemyCombatComponent::OnDodgeFinished()
{
	if (OwnerCharacter)
	{
		if (UEnemyHealthComponent* HealthComponent = OwnerCharacter->FindComponentByClass<UEnemyHealthComponent>())
		{
			if (HealthComponent->IsDeadOrOutOfHealth())
			{
				HandleOwnerDeath();
				return;
			}
		}
	}

	bIsAttacking = false;
	ActiveDodgeMontage = nullptr;
	StopManualLateralDodgeMovement();
	OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	if (Blackboard)
	{
		Blackboard->SetValueAsBool("ShouldDodge", false);
	}
	EnterState(EEnemyAIState::Chasing);
}

void UEnemyCombatComponent::HandleOwnerDeath()
{
	bIsAttacking = false;
	bComboOngoing = false;
	bShouldRotateToTarget = false;
	ComboIndex = 0;
	ComboStartIndex = 0;
	CurrentState = EEnemyAIState::Idle;
	ActiveDodgeMontage = nullptr;

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(CooldownTimer);
		GetWorld()->GetTimerManager().ClearTimer(StaminaRegenTimer);
		GetWorld()->GetTimerManager().ClearTimer(DodgeInvincibilityDelayTimer);
	}

	StopManualLateralDodgeMovement();
	EndKickDamageWindow();

	if (OwnerCharacter)
	{
		if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr)
		{
			AnimInstance->StopAllMontages(0.1f);
		}

		if (UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement())
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->DisableMovement();
		}
	}

	if (Blackboard)
	{
		Blackboard->SetValueAsBool("ShouldDodge", false);
		Blackboard->SetValueAsBool("WasHit", false);
		Blackboard->SetValueAsBool("IsInAttackRange", false);
		Blackboard->ClearValue(TargetActorKey.SelectedKeyName);
	}
}

bool UEnemyCombatComponent::TryPlayKickMontage()
{
	if (!KickMontage || !OwnerCharacter || bIsAttacking || !CanKickCurrentTarget())
	{
		return false;
	}

	FaceTargetOnce();

	if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Play(KickMontage);
		bIsAttacking = true;
		EnterState(EEnemyAIState::Attacking);

		OwnerCharacter->GetCharacterMovement()->StopMovementImmediately();
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_None);

		const float Duration = KickMontage->GetPlayLength();
		GetWorld()->GetTimerManager().SetTimer(CooldownTimer, this, &UEnemyCombatComponent::OnKickFinished, Duration, false);

		return true;
	}

	return false;
}

void UEnemyCombatComponent::OnKickFinished()
{
	bIsAttacking = false;
	EndKickDamageWindow();

	if (OwnerCharacter)
	{
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	EnterState(EEnemyAIState::Chasing);
}

void UEnemyCombatComponent::BeginKickDamageWindow()
{
	KickHitActors.Empty();
}

void UEnemyCombatComponent::TickKickDamageWindow(float DamageAmount, float HitRadius, float HitForwardOffset, float LaunchStrength, float LaunchUpwardStrength)
{
	if (!OwnerCharacter || HitRadius <= 0.0f)
	{
		return;
	}

	const FVector KickCenter = OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector() * HitForwardOffset;
	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(FName(TEXT("EnemyKick")), false, OwnerCharacter);
	if (!GetWorld()->OverlapMultiByObjectType(Overlaps, KickCenter, FQuat::Identity, ObjectQueryParams, FCollisionShape::MakeSphere(HitRadius), QueryParams))
	{
		return;
	}

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || HitActor == OwnerCharacter || KickHitActors.Contains(HitActor))
		{
			continue;
		}

		UPlayerStatsComponent* PlayerStats = HitActor->FindComponentByClass<UPlayerStatsComponent>();
		if (!PlayerStats || PlayerStats->bIsInvincible)
		{
			continue;
		}

		PlayerStats->TakeDamage(DamageAmount);
		KickHitActors.Add(HitActor);

		if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
		{
			FVector AwayFromEnemy = (HitActor->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal2D();
			if (AwayFromEnemy.IsNearlyZero())
			{
				AwayFromEnemy = OwnerCharacter->GetActorForwardVector().GetSafeNormal2D();
			}

			if (UCharacterMovementComponent* HitMovement = HitCharacter->GetCharacterMovement())
			{
				HitMovement->StopMovementImmediately();
				HitMovement->SetMovementMode(MOVE_Falling);
				HitMovement->Velocity = AwayFromEnemy * LaunchStrength + FVector::UpVector * LaunchUpwardStrength;
			}

			const FVector LaunchVelocity = AwayFromEnemy * LaunchStrength + FVector::UpVector * LaunchUpwardStrength;
			HitCharacter->LaunchCharacter(LaunchVelocity, true, true);
			StartKickKnockback(HitCharacter, AwayFromEnemy, LaunchStrength);
		}
	}
}

void UEnemyCombatComponent::EndKickDamageWindow()
{
	KickHitActors.Empty();
}
