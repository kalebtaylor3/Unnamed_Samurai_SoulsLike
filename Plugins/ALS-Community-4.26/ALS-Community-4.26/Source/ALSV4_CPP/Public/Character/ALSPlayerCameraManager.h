// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "EngineUtils.h"
#include "ALSPlayerCameraManager.generated.h"

// forward declarations
class UALSDebugComponent;
class AALSBaseCharacter;

/**
 * Player camera manager class
 */
UCLASS(Blueprintable, BlueprintType)
class ALSV4_CPP_API AALSPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	AALSPlayerCameraManager();

	UFUNCTION(BlueprintCallable, Category = "ALS|Camera")
	void OnPossess(AALSBaseCharacter* NewCharacter);

	UFUNCTION(BlueprintCallable, Category = "ALS|Camera")
	float GetCameraBehaviorParam(FName CurveName) const;

	/** Implemented debug logic in BP */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "ALS|Camera")
	void DrawDebugTargets(FVector PivotTargetLocation);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Camera")
	FVector ManualCameraOffset = FVector::ZeroVector;

	/** Used to store last known camera yaw to detect manual input */
	float LastCameraYaw = 0.f;

	/** Cooldown timer after manual camera input before auto realignment resumes */
	float ManualCameraOverrideTime = 0.f;

	/** Whether the player is actively controlling the camera */
	bool bManualOverrideActive = false;

	/** How long manual camera control disables realignment (in seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Camera|Realignment")
	float ManualCameraOverrideCooldown = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Camera|Realignment")
	float ForwardMovementDotThreshold = 0.2f;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|TargetLock")
	bool bIsTargetLocked = false;

	// Target actor to lock onto (add targeting system later)
	UPROPERTY(BlueprintReadOnly, Category = "ALS|TargetLock")
	AActor* LockedTarget = nullptr;

	// Rotation speed when locked
	UPROPERTY(EditAnywhere, Category = "ALS|TargetLock")
	float TargetLockInterpSpeed = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|TargetLock")
	FVector TargetLockCameraOffset = FVector(0.f, -30.0f, 25.f); // example: right + up

	float TargetLockPitchOffset = 0.f;

	UPROPERTY(EditAnywhere, Category = "Target Lock")
	float LockOnMaxDistance = 2000.0f;

	// Time buffer between switching targets
	UPROPERTY(EditAnywhere, Category = "Target Lock")
	float TargetSwitchCooldown = 0.15f;

	float TimeSinceLastTargetSwitch = 0.0f;

	UFUNCTION(BlueprintCallable, Category = "ALS|Camera")
	void TargetLock();

	AActor* FindTargetInDirection(bool bRight);

	void OnCameraRightInput(float Value);

	float CameraRightInputValue = 0.f;

	bool bCanSwitchTarget = true;

protected:
	virtual void UpdateViewTargetInternal(FTViewTarget& OutVT, float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "ALS|Camera")
	static FVector CalculateAxisIndependentLag(
		FVector CurrentLocation, FVector TargetLocation, FRotator CameraRotation, FVector LagSpeeds, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "ALS|Camera")
	bool CustomCameraBehavior(float DeltaTime, FVector& Location, FRotator& Rotation, float& FOV);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|Camera")
	TObjectPtr<AALSBaseCharacter> ControlledCharacter = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ALS|Camera")
	TObjectPtr<USkeletalMeshComponent> CameraBehavior = nullptr;

	UPROPERTY()
	FName TargetLockSocketName = NAME_None;

	UPROPERTY()
	USkeletalMeshComponent* LockedTargetMesh = nullptr;

	//Camera Aspect Ratio
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Camera|Filmback")
	float CameraAspectRatio = 1.777f;

	/** If true, PlayerCameraManager will force the POV to use CameraAspectRatio. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Camera|Filmback")
	bool bOverrideCameraAspectRatio = false;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Camera")
	FVector RootLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Camera")
	FTransform SmoothedPivotTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Camera")
	FVector PivotLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Camera")
	FVector TargetCameraLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ALS|Camera")
	FRotator TargetCameraRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Camera")
	FRotator DebugViewRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Camera")
	FVector DebugViewOffset;

private:
	UPROPERTY()
	TObjectPtr<UALSDebugComponent> ALSDebugComponent = nullptr;
};
