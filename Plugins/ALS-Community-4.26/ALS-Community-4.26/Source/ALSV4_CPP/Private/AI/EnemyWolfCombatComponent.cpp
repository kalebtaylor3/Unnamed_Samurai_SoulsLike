#include "AI/EnemyWolfCombatComponent.h"

#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "AI/EnemyHealthComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/PlayerStatsComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/HitResult.h"
#include "Kismet/GameplayStatics.h"

UEnemyWolfCombatComponent::UEnemyWolfCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	TargetActorKey.SelectedKeyName = "TargetActor";
}

void UEnemyWolfCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	SpawnLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;

	if (OwnerCharacter)
	{
		if (UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement())
		{
			SavedWalkSpeed = MovementComponent->MaxWalkSpeed;
			ApplyWalkSpeed(ChaseWalkSpeed);
		}

		if (AAIController* AIController = Cast<AAIController>(OwnerCharacter->GetController()))
		{
			Blackboard = AIController->GetBlackboardComponent();
		}
	}
}

void UEnemyWolfCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsDead())
	{
		return;
	}

	if (bShouldRotateToTarget && OwnerCharacter)
	{
		if (RotationInterpRemainingTime > 0.0f)
		{
			const float Alpha = 1.0f - (RotationInterpRemainingTime / RotationInterpDuration);
			OwnerCharacter->SetActorRotation(FMath::Lerp(RotationStart, RotationTarget, Alpha));
			RotationInterpRemainingTime -= DeltaTime;
		}
		else
		{
			OwnerCharacter->SetActorRotation(RotationTarget);
			bShouldRotateToTarget = false;
		}
	}

	if (!bIsAttacking && CurrentState != EWolfAIState::Circling && Blackboard)
	{
		AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
		EnterState(Target ? EWolfAIState::Chasing : EWolfAIState::Idle);
	}

	if (CurrentState == EWolfAIState::Circling && bFaceTargetWhileCircling)
	{
		FaceTargetDuringCircle(DeltaTime);
	}

	if (CurrentState == EWolfAIState::Shooting && bFaceTargetWhileLowHealthShooting)
	{
		FaceTargetDuringLowHealthShot(DeltaTime);
	}

	RequestLowHealthShotIfNeeded();
}

void UEnemyWolfCombatComponent::PerformAttack()
{
	if (!OwnerCharacter || bIsAttacking || AttackMontages.Num() == 0 || IsDead())
	{
		return;
	}

	if (!CanAttackCurrentTarget())
	{
		EnterState(EWolfAIState::Chasing);
		if (Blackboard)
		{
			Blackboard->SetValueAsBool(IsInAttackRangeKeyName, false);
		}
		return;
	}

	UAnimMontage* SelectedMontage = ChooseAttackMontage();
	if (!SelectedMontage)
	{
		return;
	}

	FaceTargetOnce();

	bIsAttacking = true;
	ActiveAttackMontage = SelectedMontage;
	EnterState(EWolfAIState::Attacking);

	if (AAIController* AIController = Cast<AAIController>(OwnerCharacter->GetController()))
	{
		AIController->StopMovement();
	}

	if (UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		ApplyWalkSpeed(AttackWalkSpeed);
	}

	if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr)
	{
		AnimInstance->Montage_Play(SelectedMontage);
		const float Duration = FMath::Max(SelectedMontage->GetPlayLength() + AttackRecoveryTime, 0.1f);
		GetWorld()->GetTimerManager().SetTimer(AttackFinishedTimer, this, &UEnemyWolfCombatComponent::OnAttackFinished, Duration, false);
	}
	else
	{
		OnAttackFinished();
	}
}

bool UEnemyWolfCombatComponent::CanAttackCurrentTarget() const
{
	const AActor* Target = Blackboard ? Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;
	return CanAttackTarget(Target);
}

bool UEnemyWolfCombatComponent::CanAttackTarget(const AActor* Target) const
{
	if (!OwnerCharacter || !Target)
	{
		return false;
	}

	return FVector::Dist2D(OwnerCharacter->GetActorLocation(), Target->GetActorLocation()) <= AttackRange;
}

bool UEnemyWolfCombatComponent::CanCircleCurrentTarget() const
{
	const AActor* Target = Blackboard ? Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;
	if (!OwnerCharacter || !Target || !GetWorld())
	{
		return false;
	}

	if (GetWorld()->GetTimeSeconds() - LastCircleTime < MinTimeBetweenCircles)
	{
		return false;
	}

	const float DistanceToTarget = FVector::Dist2D(OwnerCharacter->GetActorLocation(), Target->GetActorLocation());
	return DistanceToTarget >= MinCircleRange && DistanceToTarget <= MaxCircleRange;
}

void UEnemyWolfCombatComponent::BeginCircle()
{
	LastCircleTime = GetWorld() ? GetWorld()->GetTimeSeconds() : LastCircleTime;
	bFaceTargetWhileCircling = false;
	BeginCircleFacingLock();
	EnterState(EWolfAIState::Circling);
	ApplyWalkSpeed(CircleWalkSpeed);
}

void UEnemyWolfCombatComponent::BeginCircleHold()
{
	if (!OwnerCharacter)
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->bOrientRotationToMovement = false;
		MovementComponent->bUseControllerDesiredRotation = false;
	}

	OwnerCharacter->bUseControllerRotationYaw = false;
	bFaceTargetWhileCircling = true;
}

void UEnemyWolfCombatComponent::FinishCircle()
{
	bFaceTargetWhileCircling = false;
	StopCircleWarningMontage();
	EndCircleFacingLock();
	RestoreMovementAfterAction();
	EnterState(EWolfAIState::Chasing);
	ClearShouldCircleFlag();
}

void UEnemyWolfCombatComponent::FaceTargetOnce()
{
	if (!OwnerCharacter || !Blackboard)
	{
		return;
	}

	const AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Target)
	{
		return;
	}

	const FVector ToTarget = (Target->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal2D();
	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	RotationStart = OwnerCharacter->GetActorRotation();
	RotationTarget = FRotator(0.0f, ToTarget.Rotation().Yaw, 0.0f);
	RotationInterpRemainingTime = RotationInterpDuration;
	bShouldRotateToTarget = true;
}

void UEnemyWolfCombatComponent::FaceTargetImmediately()
{
	if (!OwnerCharacter || !Blackboard)
	{
		return;
	}

	const AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Target)
	{
		return;
	}

	const FVector ToTarget = (Target->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal2D();
	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	bShouldRotateToTarget = false;
	OwnerCharacter->SetActorRotation(FRotator(0.0f, ToTarget.Rotation().Yaw, 0.0f));
}

void UEnemyWolfCombatComponent::BeginBiteDamageWindow(float DamageAmount, float HitRadius, float HitForwardOffset)
{
	BiteHitActors.Empty();
	TickBiteDamageWindow(DamageAmount, HitRadius, HitForwardOffset);
}

void UEnemyWolfCombatComponent::TickBiteDamageWindow(float DamageAmount, float HitRadius, float HitForwardOffset)
{
	if (!OwnerCharacter || HitRadius <= 0.0f || DamageAmount <= 0.0f || IsDead())
	{
		return;
	}

	const FVector HitCenter = OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector() * HitForwardOffset;

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(FName(TEXT("EnemyWolfBite")), false, OwnerCharacter);
	if (!GetWorld()->OverlapMultiByObjectType(Overlaps, HitCenter, FQuat::Identity, ObjectQueryParams, FCollisionShape::MakeSphere(HitRadius), QueryParams))
	{
		return;
	}

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || HitActor == OwnerCharacter || BiteHitActors.Contains(HitActor))
		{
			continue;
		}

		UPlayerStatsComponent* PlayerStats = HitActor->FindComponentByClass<UPlayerStatsComponent>();
		if (!PlayerStats || PlayerStats->bIsInvincible)
		{
			continue;
		}

		PlayerStats->TakeDamage(DamageAmount);
		BiteHitActors.Add(HitActor);
	}
}

void UEnemyWolfCombatComponent::EndBiteDamageWindow()
{
	BiteHitActors.Empty();
}

void UEnemyWolfCombatComponent::HandleOwnerHit(AActor* InstigatorActor)
{
	if (!OwnerCharacter || IsDead())
	{
		return;
	}

	bIsAttacking = false;
	bShouldRotateToTarget = false;
	bFaceTargetWhileCircling = false;
	bFaceTargetWhileLowHealthShooting = false;
	bLowHealthShotCommitted = false;
	ActiveAttackMontage = nullptr;
	EndBiteDamageWindow();
	StopCircleWarningMontage();
	StopLowHealthShotMontage();
	EndCircleFacingLock();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AttackFinishedTimer);
		GetWorld()->GetTimerManager().ClearTimer(WasHitResetTimer);
		GetWorld()->GetTimerManager().ClearTimer(LowHealthShotTraceTimer);
	}

	RestoreMovementAfterAction();

	AActor* TargetActor = InstigatorActor ? InstigatorActor : UGameplayStatics::GetPlayerCharacter(this, 0);
	if (Blackboard)
	{
		if (TargetActor)
		{
			Blackboard->SetValueAsObject(TargetActorKey.SelectedKeyName, TargetActor);
		}
		Blackboard->SetValueAsBool(WasHitKeyName, true);
	}

	FaceTargetOnce();
	EnterState(EWolfAIState::Chasing);

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			WasHitResetTimer,
			this,
			&UEnemyWolfCombatComponent::ClearWasHitFlag,
			WasHitResetDelay,
			false
		);
	}
}

void UEnemyWolfCombatComponent::HandleOwnerDeath()
{
	bIsAttacking = false;
	bShouldRotateToTarget = false;
	bFaceTargetWhileCircling = false;
	bFaceTargetWhileLowHealthShooting = false;
	bLowHealthShotCommitted = false;
	ActiveAttackMontage = nullptr;
	CurrentState = EWolfAIState::Idle;
	EndBiteDamageWindow();
	StopCircleWarningMontage();
	StopLowHealthShotMontage();
	EndCircleFacingLock();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AttackFinishedTimer);
		GetWorld()->GetTimerManager().ClearTimer(WasHitResetTimer);
		GetWorld()->GetTimerManager().ClearTimer(LowHealthShotTraceTimer);
	}

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
		Blackboard->SetValueAsBool(WasHitKeyName, false);
		Blackboard->SetValueAsBool(IsInAttackRangeKeyName, false);
		Blackboard->SetValueAsBool(ShouldCircleKeyName, false);
		Blackboard->SetValueAsBool(ShouldFleeAndShootKeyName, false);
		Blackboard->ClearValue(TargetActorKey.SelectedKeyName);
	}
}

void UEnemyWolfCombatComponent::ClearWasHitFlag()
{
	if (Blackboard)
	{
		Blackboard->SetValueAsBool(WasHitKeyName, false);
	}
}

void UEnemyWolfCombatComponent::ClearShouldCircleFlag()
{
	if (Blackboard)
	{
		Blackboard->SetValueAsBool(ShouldCircleKeyName, false);
	}
}

void UEnemyWolfCombatComponent::ClearShouldFleeAndShootFlag()
{
	if (Blackboard)
	{
		Blackboard->SetValueAsBool(ShouldFleeAndShootKeyName, false);
	}
}

bool UEnemyWolfCombatComponent::IsFacingTargetForCircleWarning() const
{
	if (!OwnerCharacter || !Blackboard)
	{
		return false;
	}

	const AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Target)
	{
		return false;
	}

	const FVector ToTarget = (Target->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal2D();
	const FVector Forward = OwnerCharacter->GetActorForwardVector().GetSafeNormal2D();
	if (ToTarget.IsNearlyZero() || Forward.IsNearlyZero())
	{
		return false;
	}

	const float Dot = FMath::Clamp(FVector::DotProduct(Forward, ToTarget), -1.0f, 1.0f);
	const float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(Dot));
	return AngleDegrees <= CircleWarningFacingAngleTolerance;
}

float UEnemyWolfCombatComponent::PlayCircleWarningMontage()
{
	if (!OwnerCharacter || !CircleWarningMontage)
	{
		return 0.0f;
	}

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		return 0.0f;
	}

	const float MontageDuration = AnimInstance->Montage_Play(CircleWarningMontage, CircleWarningMontagePlayRate);
	return MontageDuration > 0.0f ? MontageDuration : CircleWarningMontage->GetPlayLength();
}

void UEnemyWolfCombatComponent::StopCircleWarningMontage()
{
	if (!OwnerCharacter || !CircleWarningMontage)
	{
		return;
	}

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInstance && AnimInstance->Montage_IsPlaying(CircleWarningMontage))
	{
		AnimInstance->Montage_Stop(CircleWarningMontageBlendOutTime, CircleWarningMontage);
	}
}

bool UEnemyWolfCombatComponent::WantsLowHealthFleeAndShoot() const
{
	if (!OwnerCharacter || !Blackboard || !LowHealthShotMontage || IsDead())
	{
		return false;
	}

	if (CurrentState == EWolfAIState::Shooting && bLowHealthShotCommitted)
	{
		return true;
	}

	if (bIsAttacking && CurrentState != EWolfAIState::Fleeing && CurrentState != EWolfAIState::Shooting)
	{
		return false;
	}

	if (CanAttackCurrentTarget())
	{
		return false;
	}

	const AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Target)
	{
		return false;
	}

	const UEnemyHealthComponent* HealthComponent = OwnerCharacter->FindComponentByClass<UEnemyHealthComponent>();
	if (!HealthComponent || HealthComponent->MaxHealth <= 0.0f)
	{
		return false;
	}

	const float HealthPercent = HealthComponent->CurrentHealth / HealthComponent->MaxHealth;
	if (HealthPercent > LowHealthShotHealthPercent)
	{
		return false;
	}

	return true;
}

bool UEnemyWolfCombatComponent::IsLowHealthShotReady() const
{
	if (CurrentState == EWolfAIState::Shooting && bLowHealthShotCommitted)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World || World->GetTimeSeconds() - LastLowHealthShotTime < LowHealthShotCooldown)
	{
		return false;
	}

	return WantsLowHealthFleeAndShoot();
}

bool UEnemyWolfCombatComponent::CanUseLowHealthShot() const
{
	return IsLowHealthShotReady();
}

void UEnemyWolfCombatComponent::BeginLowHealthFlee()
{
	bIsAttacking = true;
	bShouldRotateToTarget = false;
	bFaceTargetWhileCircling = false;
	bFaceTargetWhileLowHealthShooting = false;
	bLowHealthShotCommitted = false;
	StopCircleWarningMontage();
	EndCircleFacingLock();
	BeginCircleFacingLock();
	EnterState(EWolfAIState::Fleeing);
	ApplyWalkSpeed(LowHealthShotMoveSpeed);
}

void UEnemyWolfCombatComponent::BeginLowHealthShootFacing()
{
	if (!OwnerCharacter)
	{
		return;
	}

	if (AAIController* AIController = Cast<AAIController>(OwnerCharacter->GetController()))
	{
		AIController->StopMovement();
	}

	if (UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->bOrientRotationToMovement = false;
		MovementComponent->bUseControllerDesiredRotation = false;
	}

	OwnerCharacter->bUseControllerRotationYaw = false;
	bFaceTargetWhileLowHealthShooting = true;
	EnterState(EWolfAIState::Shooting);
}

float UEnemyWolfCombatComponent::PlayLowHealthShotMontage()
{
	if (!OwnerCharacter || !LowHealthShotMontage)
	{
		return 0.0f;
	}

	bLowHealthShotCommitted = true;

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr;
	const float MontageDuration = AnimInstance
		? AnimInstance->Montage_Play(LowHealthShotMontage, LowHealthShotMontagePlayRate)
		: 0.0f;

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(LowHealthShotTraceTimer);
		GetWorld()->GetTimerManager().SetTimer(
			LowHealthShotTraceTimer,
			this,
			&UEnemyWolfCombatComponent::PerformLowHealthShotTrace,
			LowHealthShotDelay,
			false
		);
	}

	return MontageDuration > 0.0f ? MontageDuration : LowHealthShotMontage->GetPlayLength();
}

float UEnemyWolfCombatComponent::BeginLowHealthShoot()
{
	BeginLowHealthShootFacing();
	return PlayLowHealthShotMontage();
}

void UEnemyWolfCombatComponent::FinishLowHealthShoot()
{
	bIsAttacking = false;
	bFaceTargetWhileLowHealthShooting = false;
	if (bLowHealthShotCommitted)
	{
		LastLowHealthShotTime = GetWorld() ? GetWorld()->GetTimeSeconds() : LastLowHealthShotTime;
	}
	bLowHealthShotCommitted = false;
	StopLowHealthShotMontage();
	EndCircleFacingLock();
	RestoreMovementAfterAction();
	EnterState(EWolfAIState::Chasing);
	if (Blackboard)
	{
		Blackboard->SetValueAsBool(ShouldFleeAndShootKeyName, WantsLowHealthFleeAndShoot());
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(LowHealthShotTraceTimer);
	}
}

bool UEnemyWolfCombatComponent::IsFacingTargetForLowHealthShot() const
{
	if (!OwnerCharacter || !Blackboard)
	{
		return false;
	}

	const AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Target)
	{
		return false;
	}

	const FVector ToTarget = (Target->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal2D();
	const FVector Forward = OwnerCharacter->GetActorForwardVector().GetSafeNormal2D();
	if (ToTarget.IsNearlyZero() || Forward.IsNearlyZero())
	{
		return false;
	}

	const float Dot = FMath::Clamp(FVector::DotProduct(Forward, ToTarget), -1.0f, 1.0f);
	const float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(Dot));
	return AngleDegrees <= LowHealthShotFacingAngleTolerance;
}

void UEnemyWolfCombatComponent::EnterState(EWolfAIState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	CurrentState = NewState;

	switch (CurrentState)
	{
	case EWolfAIState::Circling:
		ApplyWalkSpeed(CircleWalkSpeed);
		break;
	case EWolfAIState::Attacking:
		ApplyWalkSpeed(AttackWalkSpeed);
		break;
	case EWolfAIState::Fleeing:
		ApplyWalkSpeed(LowHealthShotMoveSpeed);
		break;
	case EWolfAIState::Shooting:
		ApplyWalkSpeed(0.0f);
		break;
	case EWolfAIState::Chasing:
		ApplyWalkSpeed(ChaseWalkSpeed);
		break;
	case EWolfAIState::Idle:
	default:
		RestoreMovementAfterAction();
		break;
	}
}

void UEnemyWolfCombatComponent::OnAttackFinished()
{
	bIsAttacking = false;
	ActiveAttackMontage = nullptr;
	EndBiteDamageWindow();
	RestoreMovementAfterAction();

	AActor* Target = Blackboard ? Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;
	if (Target && ShouldCircleAfterAttack(Target))
	{
		RequestCircle();
	}

	EnterState(Target ? EWolfAIState::Chasing : EWolfAIState::Idle);
}

UAnimMontage* UEnemyWolfCombatComponent::ChooseAttackMontage()
{
	if (AttackMontages.Num() == 0)
	{
		return nullptr;
	}

	if (AttackMontages.Num() == 1)
	{
		LastAttackIndex = 0;
		return AttackMontages[0];
	}

	int32 ChosenIndex = LastAttackIndex;
	for (int32 Attempt = 0; Attempt < 6 && ChosenIndex == LastAttackIndex; ++Attempt)
	{
		ChosenIndex = FMath::RandRange(0, AttackMontages.Num() - 1);
	}

	LastAttackIndex = ChosenIndex;
	return AttackMontages.IsValidIndex(ChosenIndex) ? AttackMontages[ChosenIndex] : nullptr;
}

bool UEnemyWolfCombatComponent::ShouldCircleAfterAttack(const AActor* Target) const
{
	if (!OwnerCharacter || !Target || !GetWorld())
	{
		return false;
	}

	if (FMath::FRand() > CircleAfterAttackChance)
	{
		return false;
	}

	if (GetWorld()->GetTimeSeconds() - LastCircleTime < MinTimeBetweenCircles)
	{
		return false;
	}

	const float DistanceToTarget = FVector::Dist2D(OwnerCharacter->GetActorLocation(), Target->GetActorLocation());
	return DistanceToTarget >= MinCircleRange && DistanceToTarget <= MaxCircleRange;
}

void UEnemyWolfCombatComponent::RequestCircle()
{
	if (Blackboard)
	{
		Blackboard->SetValueAsBool(ShouldCircleKeyName, true);
	}
}

void UEnemyWolfCombatComponent::FaceTargetDuringCircle(float DeltaTime)
{
	if (!OwnerCharacter || !Blackboard)
	{
		return;
	}

	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Target)
	{
		return;
	}

	if (AAIController* AIController = Cast<AAIController>(OwnerCharacter->GetController()))
	{
		AIController->SetFocus(Target);
	}

	const FVector ToTarget = (Target->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal2D();
	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	const FRotator DesiredRotation(0.0f, ToTarget.Rotation().Yaw, 0.0f);
	const FRotator NewRotation = FMath::RInterpTo(
		OwnerCharacter->GetActorRotation(),
		DesiredRotation,
		DeltaTime,
		CircleFaceTargetInterpSpeed
	);

	OwnerCharacter->SetActorRotation(NewRotation);
}

void UEnemyWolfCombatComponent::FaceTargetDuringLowHealthShot(float DeltaTime)
{
	FaceTargetDuringCircle(DeltaTime);
}

void UEnemyWolfCombatComponent::BeginCircleFacingLock()
{
	if (!OwnerCharacter)
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement())
	{
		if (!bHasSavedCircleRotationSettings)
		{
			bSavedOrientRotationToMovement = MovementComponent->bOrientRotationToMovement;
			bSavedUseControllerDesiredRotation = MovementComponent->bUseControllerDesiredRotation;
			bSavedUseControllerRotationYaw = OwnerCharacter->bUseControllerRotationYaw;
			bHasSavedCircleRotationSettings = true;
		}

		MovementComponent->bOrientRotationToMovement = true;
		MovementComponent->bUseControllerDesiredRotation = false;
		OwnerCharacter->bUseControllerRotationYaw = false;
	}
}

void UEnemyWolfCombatComponent::EndCircleFacingLock()
{
	if (!OwnerCharacter || !bHasSavedCircleRotationSettings)
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = bSavedOrientRotationToMovement;
		MovementComponent->bUseControllerDesiredRotation = bSavedUseControllerDesiredRotation;
		OwnerCharacter->bUseControllerRotationYaw = bSavedUseControllerRotationYaw;
	}

	bHasSavedCircleRotationSettings = false;
}

void UEnemyWolfCombatComponent::RestoreMovementAfterAction()
{
	if (UCharacterMovementComponent* MovementComponent = OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr)
	{
		MovementComponent->SetMovementMode(MOVE_Walking);
		MovementComponent->MaxWalkSpeed = SavedWalkSpeed > 0.0f ? SavedWalkSpeed : ChaseWalkSpeed;
	}
}

void UEnemyWolfCombatComponent::ApplyWalkSpeed(float NewWalkSpeed)
{
	if (UCharacterMovementComponent* MovementComponent = OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr)
	{
		MovementComponent->MaxWalkSpeed = NewWalkSpeed;
	}
}

void UEnemyWolfCombatComponent::RequestLowHealthShotIfNeeded()
{
	if (Blackboard)
	{
		if (CurrentState == EWolfAIState::Shooting && bLowHealthShotCommitted)
		{
			Blackboard->SetValueAsBool(ShouldFleeAndShootKeyName, true);
			return;
		}

		if (CurrentState == EWolfAIState::Fleeing)
		{
			Blackboard->SetValueAsBool(ShouldFleeAndShootKeyName, !CanAttackCurrentTarget());
			return;
		}

		Blackboard->SetValueAsBool(ShouldFleeAndShootKeyName, WantsLowHealthFleeAndShoot());
	}
}

void UEnemyWolfCombatComponent::PerformLowHealthShotTrace()
{
	if (!OwnerCharacter || LowHealthShotRange <= 0.0f || LowHealthShotDamage <= 0.0f || IsDead())
	{
		return;
	}

	const FVector Start = OwnerCharacter->GetActorLocation()
		+ FVector::UpVector * LowHealthShotTraceHeightOffset
		+ OwnerCharacter->GetActorForwardVector() * LowHealthShotTraceForwardOffset;
	const FVector End = Start + OwnerCharacter->GetActorForwardVector() * LowHealthShotRange;

	TArray<FHitResult> Hits;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(FName(TEXT("EnemyWolfLowHealthShot")), false, OwnerCharacter);
	if (!GetWorld() || !GetWorld()->LineTraceMultiByObjectType(Hits, Start, End, ObjectQueryParams, QueryParams))
	{
		return;
	}

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == OwnerCharacter)
		{
			continue;
		}

		UPlayerStatsComponent* PlayerStats = HitActor->FindComponentByClass<UPlayerStatsComponent>();
		if (!PlayerStats || PlayerStats->bIsInvincible)
		{
			continue;
		}

		PlayerStats->TakeDamage(LowHealthShotDamage);
		return;
	}
}

void UEnemyWolfCombatComponent::StopLowHealthShotMontage()
{
	if (!OwnerCharacter || !LowHealthShotMontage)
	{
		return;
	}

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInstance && AnimInstance->Montage_IsPlaying(LowHealthShotMontage))
	{
		AnimInstance->Montage_Stop(LowHealthShotMontageBlendOutTime, LowHealthShotMontage);
	}
}

bool UEnemyWolfCombatComponent::IsDead() const
{
	const UEnemyHealthComponent* HealthComponent = OwnerCharacter ? OwnerCharacter->FindComponentByClass<UEnemyHealthComponent>() : nullptr;
	return HealthComponent && HealthComponent->IsDeadOrOutOfHealth();
}
