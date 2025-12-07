// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community


#include "Character/ALSPlayerCameraManager.h"

#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Character/ALSBaseCharacter.h"
#include "Character/ALSPlayerController.h"
#include "Character/Animation/ALSPlayerCameraBehavior.h"
#include "Character/UI/LockOnWidgetComponent.h"
#include "AI/EnemyHealthBarWidgetComponent.h"
#include "Components/ALSDebugComponent.h"
#include "AI/EnemyHealthComponent.h"
#include "Kismet/KismetMathLibrary.h"


const FName NAME_CameraBehavior(TEXT("CameraBehavior"));
const FName NAME_CameraOffset_X(TEXT("CameraOffset_X"));
const FName NAME_CameraOffset_Y(TEXT("CameraOffset_Y"));
const FName NAME_CameraOffset_Z(TEXT("CameraOffset_Z"));
const FName NAME_Override_Debug(TEXT("Override_Debug"));
const FName NAME_PivotLagSpeed_X(TEXT("PivotLagSpeed_X"));
const FName NAME_PivotLagSpeed_Y(TEXT("PivotLagSpeed_Y"));
const FName NAME_PivotLagSpeed_Z(TEXT("PivotLagSpeed_Z"));
const FName NAME_PivotOffset_X(TEXT("PivotOffset_X"));
const FName NAME_PivotOffset_Y(TEXT("PivotOffset_Y"));
const FName NAME_PivotOffset_Z(TEXT("PivotOffset_Z"));
const FName NAME_RotationLagSpeed(TEXT("RotationLagSpeed"));
const FName NAME_Weight_FirstPerson(TEXT("Weight_FirstPerson"));


AALSPlayerCameraManager::AALSPlayerCameraManager()
{
	CameraBehavior = CreateDefaultSubobject<USkeletalMeshComponent>(NAME_CameraBehavior);
	CameraBehavior->SetupAttachment(GetRootComponent());
	CameraBehavior->bHiddenInGame = true;
}

void AALSPlayerCameraManager::OnPossess(AALSBaseCharacter* NewCharacter)
{
	// Set "Controlled Pawn" when Player Controller Possesses new character. (called from Player Controller)
	check(NewCharacter);
	ControlledCharacter = NewCharacter;

	// Update references in the Camera Behavior AnimBP.
	UALSPlayerCameraBehavior* CastedBehv = Cast<UALSPlayerCameraBehavior>(CameraBehavior->GetAnimInstance());
	if (CastedBehv)
	{
		NewCharacter->SetCameraBehavior(CastedBehv);
		CastedBehv->MovementState = NewCharacter->GetMovementState();
		CastedBehv->MovementAction = NewCharacter->GetMovementAction();
		CastedBehv->bRightShoulder = NewCharacter->IsRightShoulder();
		CastedBehv->Gait = NewCharacter->GetGait();
		CastedBehv->SetRotationMode(NewCharacter->GetRotationMode());
		CastedBehv->Stance = NewCharacter->GetStance();
		CastedBehv->ViewMode = NewCharacter->GetViewMode();
	}

	// Initial position
	const FVector& TPSLoc = ControlledCharacter->GetThirdPersonPivotTarget().GetLocation();
	SetActorLocation(TPSLoc);
	SmoothedPivotTarget.SetLocation(TPSLoc);

	ALSDebugComponent = ControlledCharacter->FindComponentByClass<UALSDebugComponent>();
}

float AALSPlayerCameraManager::GetCameraBehaviorParam(FName CurveName) const
{
	UAnimInstance* Inst = CameraBehavior->GetAnimInstance();
	if (Inst)
	{
		return Inst->GetCurveValue(CurveName);
	}
	return 0.0f;
}

void AALSPlayerCameraManager::TargetLock()
{
	if (!ControlledCharacter) return;

	bIsTargetLocked = !bIsTargetLocked;

	if (bIsTargetLocked)
	{
		AActor* BestTarget = nullptr;
		float ClosestScore = FLT_MAX;

		const FVector PlayerLocation = ControlledCharacter->GetActorLocation();
		const FVector PlayerViewDir = ControlledCharacter->GetControlRotation().Vector();

		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			AActor* Candidate = *It;
			if (!Candidate || Candidate == ControlledCharacter || !Candidate->ActorHasTag("Enemy")) continue;

			if (UEnemyHealthComponent* HealthComp = Candidate->FindComponentByClass<UEnemyHealthComponent>())
			{
				if (HealthComp->bIsDead) continue;
			}

			const FVector EnemyLocation = Candidate->GetActorLocation();
			const float Distance = FVector::Dist(PlayerLocation, EnemyLocation);
			if (Distance > LockOnMaxDistance) continue;

			// Visibility check
			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(ControlledCharacter);
			Params.AddIgnoredActor(Candidate);

			bool bBlocked = GetWorld()->LineTraceSingleByChannel(
				Hit,
				PlayerLocation + FVector(0, 0, 50), // eye level
				EnemyLocation,
				ECC_Visibility,
				Params
			);

			if (bBlocked) continue;

			// Score: weighted sum of distance and angle
			const FVector ToTarget = (EnemyLocation - PlayerLocation).GetSafeNormal();
			const float AngleScore = 1.0f - FVector::DotProduct(PlayerViewDir, ToTarget); // 0 is directly forward, 1 is behind
			const float TotalScore = Distance + (AngleScore * 500.f); // weight angle more

			if (TotalScore < ClosestScore)
			{
				ClosestScore = TotalScore;
				BestTarget = Candidate;
			}
		}

		if (BestTarget)
		{
			LockedTarget = BestTarget;
			LockedTargetMesh = BestTarget->FindComponentByClass<USkeletalMeshComponent>();
			TargetLockSocketName = "TargetLockSocket";

			if (ULockOnWidgetComponent* LockWidget = BestTarget->FindComponentByClass<ULockOnWidgetComponent>())
			{
				LockWidget->TargetPlayer = ControlledCharacter->GetController<APlayerController>();
				LockWidget->SetVisibility(true);
			}

			if (UEnemyHealthBarWidgetComponent* HealthBar = BestTarget->FindComponentByClass<UEnemyHealthBarWidgetComponent>())
			{
				HealthBar->SetVisibility(true);
			}

			ControlledCharacter->LookingDirectionAction();
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Target Lock: ON"));
		}
		else
		{
			bIsTargetLocked = false;
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("No valid target found"));
		}
	}
	else
	{
		if (LockedTarget)
		{
			if (ULockOnWidgetComponent* LockWidget = LockedTarget->FindComponentByClass<ULockOnWidgetComponent>())
			{
				LockWidget->SetVisibility(false);
			}

			if (UEnemyHealthBarWidgetComponent* HealthBar = LockedTarget->FindComponentByClass<UEnemyHealthBarWidgetComponent>())
			{
				HealthBar->SetVisibility(false);
			}
		}

		ControlledCharacter->VelocityDirectionAction();
		LockedTarget = nullptr;
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Target Lock: OFF"));
	}
}

void AALSPlayerCameraManager::UpdateViewTargetInternal(FTViewTarget& OutVT, float DeltaTime)
{
	// Partially taken from base class

	if (OutVT.Target)
	{
		FVector OutLocation;
		FRotator OutRotation;
		float OutFOV;

		if (OutVT.Target->IsA<AALSBaseCharacter>())
		{
			if (CustomCameraBehavior(DeltaTime, OutLocation, OutRotation, OutFOV))
			{
				OutVT.POV.Location = OutLocation;
				OutVT.POV.Rotation = OutRotation;
				OutVT.POV.FOV      = OutFOV;

				// 🔻 NEW: Aspect / filmback control
				if (bOverrideCameraAspectRatio && CameraAspectRatio > 0.f)
				{
					OutVT.POV.AspectRatio = CameraAspectRatio;
					OutVT.POV.bConstrainAspectRatio = true;
				}
			}
			else
			{
				OutVT.Target->CalcCamera(DeltaTime, OutVT.POV);
			}
		}
		else
		{
			OutVT.Target->CalcCamera(DeltaTime, OutVT.POV);
		}
	}
}

FVector AALSPlayerCameraManager::CalculateAxisIndependentLag(FVector CurrentLocation, FVector TargetLocation,
                                                             FRotator CameraRotation, FVector LagSpeeds,
                                                             float DeltaTime)
{
	CameraRotation.Roll = 0.0f;
	CameraRotation.Pitch = 0.0f;
	const FVector UnrotatedCurLoc = CameraRotation.UnrotateVector(CurrentLocation);
	const FVector UnrotatedTargetLoc = CameraRotation.UnrotateVector(TargetLocation);

	const FVector ResultVector(
		FMath::FInterpTo(UnrotatedCurLoc.X, UnrotatedTargetLoc.X, DeltaTime, LagSpeeds.X),
		FMath::FInterpTo(UnrotatedCurLoc.Y, UnrotatedTargetLoc.Y, DeltaTime, LagSpeeds.Y),
		FMath::FInterpTo(UnrotatedCurLoc.Z, UnrotatedTargetLoc.Z, DeltaTime, LagSpeeds.Z));

	return CameraRotation.RotateVector(ResultVector);
}

bool AALSPlayerCameraManager::CustomCameraBehavior(float DeltaTime, FVector& Location, FRotator& Rotation, float& FOV)
{
	if (!ControlledCharacter)
	{
		return false;
	}

	const FTransform& PivotTarget = ControlledCharacter->GetThirdPersonPivotTarget();
	const FVector& FPTarget = ControlledCharacter->GetFirstPersonCameraTarget();
	float TPFOV = 90.0f;
	float FPFOV = 90.0f;
	bool bRightShoulder = false;
	ControlledCharacter->GetCameraParameters(TPFOV, FPFOV, bRightShoulder);

	AALSPlayerController* PC = Cast<AALSPlayerController>(GetOwningPlayerController());
	FRotator ControlRot = PC->GetControlRotation(); // mutable
	const FVector VelocityDir = ControlledCharacter->GetVelocity().GetSafeNormal2D();

	// Manual override check
	float CurrentYaw = ControlRot.Yaw;
	float YawDelta = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentYaw, LastCameraYaw));
	LastCameraYaw = CurrentYaw;

	if (YawDelta > 2.5f)
	{
		ManualCameraOverrideTime = ManualCameraOverrideCooldown;
		bManualOverrideActive = true;
	}
	else if (ManualCameraOverrideTime > 0.f)
	{
		ManualCameraOverrideTime -= DeltaTime;
		if (ManualCameraOverrideTime <= 0.f)
		{
			bManualOverrideActive = false;
		}
	}

	// Lock rotation override
	if (bIsTargetLocked && LockedTarget)
	{
		FVector ToTarget = (LockedTarget->GetActorLocation() - ControlledCharacter->GetActorLocation()).GetSafeNormal2D();
		FRotator TargetRot = ToTarget.Rotation();
		ControlRot = FMath::RInterpTo(ControlRot, TargetRot, DeltaTime, TargetLockInterpSpeed);
		PC->SetControlRotation(ControlRot);
	}
	else if (ControlledCharacter->GetRotationMode() == EALSRotationMode::VelocityDirection)
	{
		const FVector ForwardVector = UKismetMathLibrary::GetForwardVector(ControlRot);
		const float ForwardDot = FVector::DotProduct(ForwardVector, VelocityDir);
		const bool bIsMovingForward = ForwardDot > ForwardMovementDotThreshold;

		if (bIsMovingForward && !bManualOverrideActive)
		{
			const FRotator TargetYawRot = VelocityDir.ToOrientationRotator();
			const float AngleToTarget = FMath::Abs(FMath::FindDeltaAngleDegrees(ControlRot.Yaw, TargetYawRot.Yaw));

			if (AngleToTarget > 4.f)
			{
				float RealignSpeed = (ControlledCharacter->GetGait() == EALSGait::Sprinting ? 1.5f : 1.0f);
				FRotator SmoothedRot = FMath::RInterpTo(ControlRot, TargetYawRot, DeltaTime, RealignSpeed);

				if (FMath::Abs(FMath::FindDeltaAngleDegrees(ControlRot.Yaw, SmoothedRot.Yaw)) > 0.1f)
				{
					PC->SetControlRotation(SmoothedRot);
					ControlRot = SmoothedRot;
				}
			}
		}
	}

	FRotator FinalCamRot;

	if (bIsTargetLocked && LockedTarget)
	{
		// === FINAL ROTATION: Use ControlRot.Yaw but apply pitch manually ===
		FVector TargetLocation = LockedTarget->GetActorLocation();

		if (LockedTargetMesh && TargetLockSocketName != NAME_None)
		{
			const FTransform MeshTransform = LockedTargetMesh->GetComponentTransform();
			const FVector SocketRelativeLocation = LockedTargetMesh->GetSocketTransform(TargetLockSocketName, RTS_ParentBoneSpace).GetLocation();

			TargetLocation = MeshTransform.TransformPosition(SocketRelativeLocation);
		}

		const FVector ToTarget = TargetLocation - ControlledCharacter->GetActorLocation();
		const float Distance = ToTarget.Size();

		const float TiltMinDistance = 150.f;
		const float TiltMaxDistance = 1000.f;
		const float MaxTiltUpDegrees = 50.f;
		const float MaxTiltDownDegrees = 35.f;

		float TiltAlpha = 1.f - ((FMath::Clamp(Distance, TiltMinDistance, TiltMaxDistance) - TiltMinDistance) / (TiltMaxDistance - TiltMinDistance));
		const float VerticalRatio = ToTarget.Z / Distance; // How vertical is the target
		float DesiredTilt = 0.f;

		if (VerticalRatio >= 0.f)
		{
			// Target is above — tilt up more
			DesiredTilt = FMath::Clamp(VerticalRatio * MaxTiltUpDegrees, 0.f, MaxTiltUpDegrees);
		}
		else
		{
			// Target is below — tilt down less
			DesiredTilt = FMath::Clamp(VerticalRatio * MaxTiltDownDegrees, -MaxTiltDownDegrees, 0.f);
		}

		FinalCamRot = ControlRot;
		FinalCamRot.Pitch = FMath::Clamp(DesiredTilt, -89.f, 89.f); // override pitch entirely

		TimeSinceLastTargetSwitch += DeltaTime;

		if (FMath::Abs(CameraRightInputValue) >= 0.4f && TimeSinceLastTargetSwitch > TargetSwitchCooldown)
		{
			const bool bSwitchRight = CameraRightInputValue > 0.f;
			AActor* NewTarget = FindTargetInDirection(bSwitchRight);

			if (NewTarget && NewTarget != LockedTarget)
			{
				// 🔻 Hide widgets on old target
				if (LockedTarget)
				{
					if (ULockOnWidgetComponent* OldWidget = LockedTarget->FindComponentByClass<ULockOnWidgetComponent>())
					{
						OldWidget->SetVisibility(false);
					}

					if (UEnemyHealthBarWidgetComponent* OldHealthBar = LockedTarget->FindComponentByClass<UEnemyHealthBarWidgetComponent>())
					{
						OldHealthBar->SetVisibility(false);
					}
				}

				// 🔺 Show widgets on new target
				if (ULockOnWidgetComponent* NewWidget = NewTarget->FindComponentByClass<ULockOnWidgetComponent>())
				{
					NewWidget->TargetPlayer = ControlledCharacter->GetController<APlayerController>();
					NewWidget->SetVisibility(true);
				}

				if (UEnemyHealthBarWidgetComponent* NewHealthBar = NewTarget->FindComponentByClass<UEnemyHealthBarWidgetComponent>())
				{
					NewHealthBar->SetVisibility(true);
				}

				// Update lock state
				LockedTarget = NewTarget;
				LockedTargetMesh = LockedTarget->FindComponentByClass<USkeletalMeshComponent>();
				TimeSinceLastTargetSwitch = 0.0f;
				CameraRightInputValue = 0.0f;

				GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan,
					FString::Printf(TEXT("Switched Target to: %s"), *NewTarget->GetName()));
			}

			TimeSinceLastTargetSwitch = 0.0f;
			CameraRightInputValue = 0.0f;
		}
	}
	else
	{
		const FRotator InterpResult = FMath::RInterpTo(GetCameraRotation(), ControlRot, DeltaTime, GetCameraBehaviorParam(NAME_RotationLagSpeed));
		FinalCamRot = UKismetMathLibrary::RLerp(InterpResult, DebugViewRotation, GetCameraBehaviorParam(NAME_Override_Debug), true);
	}

	TargetCameraRotation = FinalCamRot;

	// === Pivot smoothing ===
	const FVector LagSpd(GetCameraBehaviorParam(NAME_PivotLagSpeed_X), GetCameraBehaviorParam(NAME_PivotLagSpeed_Y), GetCameraBehaviorParam(NAME_PivotLagSpeed_Z));
	const FVector AxisIndpLag = CalculateAxisIndependentLag(SmoothedPivotTarget.GetLocation(), PivotTarget.GetLocation(), FinalCamRot, LagSpd, DeltaTime);

	SmoothedPivotTarget.SetRotation(PivotTarget.GetRotation());
	SmoothedPivotTarget.SetLocation(AxisIndpLag);

	// === Pivot Offsets ===
	PivotLocation =
		SmoothedPivotTarget.GetLocation() +
		UKismetMathLibrary::GetForwardVector(SmoothedPivotTarget.Rotator()) * GetCameraBehaviorParam(NAME_PivotOffset_X) +
		UKismetMathLibrary::GetRightVector(SmoothedPivotTarget.Rotator()) * GetCameraBehaviorParam(NAME_PivotOffset_Y) +
		UKismetMathLibrary::GetUpVector(SmoothedPivotTarget.Rotator()) * GetCameraBehaviorParam(NAME_PivotOffset_Z);

	// === Camera Offsets ===
	TargetCameraLocation = PivotLocation +
		UKismetMathLibrary::GetForwardVector(FinalCamRot) * GetCameraBehaviorParam(NAME_CameraOffset_X) +
		UKismetMathLibrary::GetRightVector(FinalCamRot) * GetCameraBehaviorParam(NAME_CameraOffset_Y) +
		UKismetMathLibrary::GetUpVector(FinalCamRot) * GetCameraBehaviorParam(NAME_CameraOffset_Z);

	if (bIsTargetLocked)
	{
		TargetCameraLocation +=
			UKismetMathLibrary::GetRightVector(FinalCamRot) * TargetLockCameraOffset.Y +
			UKismetMathLibrary::GetUpVector(FinalCamRot) * TargetLockCameraOffset.Z +
			UKismetMathLibrary::GetForwardVector(FinalCamRot) * TargetLockCameraOffset.X;
	}

	// === Collision Trace ===
	FVector TraceOrigin;
	float TraceRadius;
	ECollisionChannel TraceChannel = ControlledCharacter->GetThirdPersonTraceParams(TraceOrigin, TraceRadius);

	FHitResult HitResult;
	const FCollisionShape SphereCollisionShape = FCollisionShape::MakeSphere(TraceRadius);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(ControlledCharacter);

	bool bHit = GetWorld()->SweepSingleByChannel(HitResult, TraceOrigin, TargetCameraLocation, FQuat::Identity, TraceChannel, SphereCollisionShape, Params);

	if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
	{
		UALSDebugComponent::DrawDebugSphereTraceSingle(GetWorld(), TraceOrigin, TargetCameraLocation, SphereCollisionShape, EDrawDebugTrace::Type::ForOneFrame, bHit, HitResult, FLinearColor::Red, FLinearColor::Green, 5.0f);
	}

	if (HitResult.IsValidBlockingHit())
	{
		TargetCameraLocation += HitResult.Location - HitResult.TraceEnd;
	}

	// === Final camera transform ===
	FTransform TargetCameraTransform(FinalCamRot, TargetCameraLocation, FVector::OneVector);
	FTransform FPTargetCameraTransform(FinalCamRot, FPTarget, FVector::OneVector);

	const FTransform MixedTransform = UKismetMathLibrary::TLerp(TargetCameraTransform, FPTargetCameraTransform, GetCameraBehaviorParam(NAME_Weight_FirstPerson));
	const FTransform FinalCameraTransform = UKismetMathLibrary::TLerp(MixedTransform, FTransform(DebugViewRotation, TargetCameraLocation, FVector::OneVector), GetCameraBehaviorParam(NAME_Override_Debug));

	Location = FinalCameraTransform.GetLocation() + ManualCameraOffset;
	Rotation = FinalCameraTransform.Rotator();
	FOV = FMath::Lerp(TPFOV, FPFOV, GetCameraBehaviorParam(NAME_Weight_FirstPerson));

	return true;
}

AActor* AALSPlayerCameraManager::FindTargetInDirection(bool bRight)
{
	AActor* BestTarget = nullptr;
	float BestAngle = 180.f;

	const FVector MyLoc = ControlledCharacter->GetActorLocation();
	const FRotator ViewRot = ControlledCharacter->GetControlRotation();
	const FVector ViewForward = ViewRot.Vector();
	const FVector ViewRight = FRotationMatrix(ViewRot).GetUnitAxis(EAxis::Y); // right vector

	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Candidate = *It;
		if (!Candidate || Candidate == ControlledCharacter || !Candidate->ActorHasTag("Enemy")) continue;

		if (UEnemyHealthComponent* HealthComp = Candidate->FindComponentByClass<UEnemyHealthComponent>())
		{
			if (HealthComp->bIsDead) continue;
		}

		const float Distance = FVector::Dist(MyLoc, Candidate->GetActorLocation());
		if (Distance > LockOnMaxDistance) continue;

		const FVector ToTarget = (Candidate->GetActorLocation() - MyLoc).GetSafeNormal2D();
		const float ForwardDot = FVector::DotProduct(ViewForward, ToTarget);
		if (ForwardDot <= 0.f) continue; // behind player

		const float RightDot = FVector::DotProduct(ViewRight, ToTarget);
		const float AngleDeg = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));

		if ((bRight && AngleDeg > 0.f && AngleDeg < BestAngle) ||
			(!bRight && AngleDeg < 0.f && -AngleDeg < BestAngle))
		{
			// Line of sight check
			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(ControlledCharacter);
			Params.AddIgnoredActor(Candidate);

			bool bBlocked = GetWorld()->LineTraceSingleByChannel(
				Hit,
				MyLoc + FVector(0, 0, 50),
				Candidate->GetActorLocation(),
				ECC_Visibility,
				Params
			);

			if (!bBlocked)
			{
				BestAngle = FMath::Abs(AngleDeg);
				BestTarget = Candidate;
			}
		}
	}

	return BestTarget;
}

void AALSPlayerCameraManager::OnCameraRightInput(float Value)
{
	CameraRightInputValue = Value;
}