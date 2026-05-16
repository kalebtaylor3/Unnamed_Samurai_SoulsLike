#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseCastProjectile.generated.h"

class AALSBaseCharacter;
class UBoxComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UParticleSystem;
class UProjectileMovementComponent;
class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class ALSV4_CPP_API ABaseCastProjectile : public AActor
{
	GENERATED_BODY()

public:
	ABaseCastProjectile();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Magic Projectile")
	void InitializeMagicProjectile(AActor* InCaster, AActor* InTarget = nullptr, float OverrideDamage = -1.0f, float OverrideSpeed = -1.0f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* TrailFX;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Damage")
	float DamageAmount = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Damage")
	bool bAllowMultipleEnemyHits = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Movement")
	float InitialSpeed = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Movement")
	float MaxSpeed = 1650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Movement")
	bool bRotationFollowsVelocity = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Movement")
	float ProjectileGravityScale = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Movement")
	float MinimumSpawnHeightAboveCaster = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel")
	bool bUseMagicLaunchStyle = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel", meta = (EditCondition = "bUseMagicLaunchStyle", ClampMin = "0.0"))
	float LaunchDelay = 0.24f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel", meta = (EditCondition = "bUseMagicLaunchStyle"))
	FVector LaunchFloatOffset = FVector(12.0f, 0.0f, 22.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel", meta = (EditCondition = "bUseMagicLaunchStyle", ClampMin = "0.0"))
	float SpeedRampDuration = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel", meta = (EditCondition = "bUseMagicLaunchStyle", ClampMin = "0.0", ClampMax = "1.0"))
	float InitialLaunchSpeedRatio = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel", meta = (EditCondition = "bUseMagicLaunchStyle", ClampMin = "0.0"))
	float HomingRampDuration = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel")
	float LaunchPitchDegrees = 14.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel", meta = (ClampMin = "0.0"))
	float HomingDelayAfterLaunch = 0.22f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel")
	bool bUseVisualSpiral = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel", meta = (EditCondition = "bUseVisualSpiral", ClampMin = "0.0"))
	float SpiralAmplitude = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel", meta = (EditCondition = "bUseVisualSpiral", ClampMin = "0.0"))
	float SpiralFrequency = 7.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel", meta = (EditCondition = "bUseVisualSpiral", ClampMin = "0.0"))
	float SpiralFadeInDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Homing")
	bool bUseHoming = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Homing")
	bool bAlwaysHeatSeekEnemies = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Homing")
	float HomingAccelerationMagnitude = 3200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Homing")
	float HomingTurnSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Homing")
	bool bContinuouslyAcquireTargets = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Homing", meta = (EditCondition = "bContinuouslyAcquireTargets", ClampMin = "0.01"))
	float TargetAcquireInterval = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Homing")
	bool bPreferLockedTarget = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Homing")
	bool bAutoFindTarget = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Homing")
	float TargetSearchRange = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Homing", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float TargetForwardDotThreshold = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Homing")
	FName TargetSocketName = TEXT("TargetLockSocket");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Homing")
	float TargetFallbackHeightOffset = 95.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Homing")
	float TrackingPredictionTime = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Homing")
	float MinimumLaunchPitch = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Homing")
	FName EnemyTargetTag = TEXT("Enemy");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Homing", meta = (ClampMin = "0.0"))
	float HeatSeekCatchRadius = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Homing")
	bool bHeatSeekAimAtCenterMass = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Homing", meta = (EditCondition = "bHeatSeekAimAtCenterMass"))
	float HeatSeekCenterMassZOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel")
	bool bUseMagicalHeatSeekPath = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel", meta = (EditCondition = "bUseMagicalHeatSeekPath", ClampMin = "0.0"))
	float HeatSeekWeaveStrength = 0.22f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel", meta = (EditCondition = "bUseMagicalHeatSeekPath", ClampMin = "0.0"))
	float HeatSeekVerticalLiftStrength = 0.06f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel", meta = (EditCondition = "bUseMagicalHeatSeekPath", ClampMin = "0.0"))
	float HeatSeekWeaveFrequency = 8.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel", meta = (EditCondition = "bUseMagicalHeatSeekPath", ClampMin = "0.0"))
	float HeatSeekCloseFadeDistance = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel", meta = (EditCondition = "bUseMagicalHeatSeekPath", ClampMin = "0.0"))
	float HeatSeekTurnSharpness = 26.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel")
	bool bUseMagicalHeatSeekSpeed = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel", meta = (EditCondition = "bUseMagicalHeatSeekSpeed", ClampMin = "0.0"))
	float HeatSeekStartSpeed = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel", meta = (EditCondition = "bUseMagicalHeatSeekSpeed", ClampMin = "0.0"))
	float HeatSeekCruiseSpeed = 760.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel", meta = (EditCondition = "bUseMagicalHeatSeekSpeed", ClampMin = "0.0"))
	float HeatSeekSpeedRampDuration = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel", meta = (EditCondition = "bUseMagicalHeatSeekSpeed", ClampMin = "0.0", ClampMax = "1.0"))
	float HeatSeekCloseSpeedMultiplier = 0.72f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel", meta = (EditCondition = "bUseMagicalHeatSeekSpeed", ClampMin = "0.0", ClampMax = "0.5"))
	float HeatSeekSpeedPulseStrength = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Magic Feel", meta = (EditCondition = "bUseMagicalHeatSeekSpeed", ClampMin = "0.0"))
	float HeatSeekSpeedPulseFrequency = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Debug")
	bool bDebugTargeting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Debug", meta = (EditCondition = "bDebugTargeting", ClampMin = "0.0"))
	float TargetDebugDrawDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Trace")
	bool bSweepBetweenFrames = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Trace")
	bool bTraceOnlyEnemiesForSweeps = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Impact")
	bool bDestroyOnEnemyHit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Impact")
	bool bDestroyOnWorldHit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Impact")
	float DestroyDelayAfterImpact = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Impact")
	UNiagaraSystem* ImpactNiagara = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Impact")
	UParticleSystem* ImpactParticle = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Impact")
	FVector ImpactFXScale = FVector(1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Impact")
	float ImpactFXSurfaceOffset = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Impact")
	bool bOrientImpactFXToSurface = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Projectile|Impact")
	bool bUseProjectileFacingForFlatImpacts = true;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintNativeEvent, Category = "Magic Projectile")
	void OnProjectileImpacted(AActor* HitActor, const FHitResult& Hit);
	virtual void OnProjectileImpacted_Implementation(AActor* HitActor, const FHitResult& Hit);

private:
	void AddIgnoredActor(AActor* ActorToIgnore);
	bool IsIgnoredActor(const AActor* Actor) const;
	bool IsValidHeatSeekTarget(AActor* Candidate) const;
	AActor* GetLockedTargetFromCaster() const;
	AActor* FindBestTarget() const;
	void DebugTargetingMessage(const FString& Message, const FColor& Color) const;
	USceneComponent* GetTargetHomingComponent(AActor* TargetActor) const;
	FVector GetTargetAimLocation(AActor* TargetActor) const;
	FVector GetHeatSeekAimLocation(AActor* TargetActor) const;
	FVector GetCasterAimForward() const;
	void RefreshHomingTarget();
	void ResetMagicLaunchState();
	void UpdateMagicLaunch(float DeltaSeconds);
	void LaunchProjectile();
	void UpdateMagicMotion(float DeltaSeconds);
	void UpdateVisualSpiral();
	bool TryCatchHeatSeekTarget(const FVector& TraceStart, const FVector& TraceEnd);
	float GetMagicalHeatSeekSpeed(float BaseSpeed, float DistanceToTarget) const;
	FVector GetMagicalHeatSeekDirection(const FVector& DirectDirection, float DistanceToTarget) const;
	FVector GetForwardLaunchDirection() const;
	FVector GetDesiredLaunchDirection() const;
	void ApplyInitialVelocity();
	void HandleActorHit(AActor* HitActor, const FHitResult& Hit);
	void HandleEnemyHit(AActor* HitActor, const FHitResult& Hit);
	void HandleWorldHit(AActor* HitActor, const FHitResult& Hit);
	void SpawnImpactFX(const FHitResult& Hit) const;
	void StopProjectile();
	void FinishProjectile();

	UPROPERTY()
	AActor* Caster = nullptr;

	UPROPERTY()
	AActor* HomingTarget = nullptr;

	UPROPERTY()
	TSet<AActor*> IgnoredActors;

	UPROPERTY()
	TSet<AActor*> DamagedActors;

	FVector PreviousLocation = FVector::ZeroVector;
	FVector SpawnLocation = FVector::ZeroVector;
	FVector SpiralRight = FVector::RightVector;
	FVector SpiralUp = FVector::UpVector;
	FRotator SpawnAimRotation = FRotator::ZeroRotator;
	float ProjectileAge = 0.0f;
	float TimeSinceLaunch = 0.0f;
	float TimeSinceTargetAcquire = 0.0f;
	float SpiralPhase = 0.0f;
	bool bHasLaunched = false;
	bool bHasImpacted = false;
};
