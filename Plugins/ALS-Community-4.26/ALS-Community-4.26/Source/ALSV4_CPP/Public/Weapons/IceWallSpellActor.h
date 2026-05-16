#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IceWallSpellActor.generated.h"

class AALSBaseCharacter;
class UNiagaraComponent;
class UNiagaraSystem;
class USceneComponent;

UCLASS(Blueprintable)
class ALSV4_CPP_API AIceWallSpellActor : public AActor
{
	GENERATED_BODY()

public:
	AIceWallSpellActor();

	UFUNCTION(BlueprintCallable, Category = "Ice Wall")
	void InitializeIceWall(AALSBaseCharacter* InCaster);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* IceWallFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|FX")
	UNiagaraSystem* IceWallNiagara = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Damage")
	float DamageAmount = 85.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Damage")
	FVector DamageBoxHalfExtent = FVector(260.0f, 115.0f, 150.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Damage")
	FVector DamageBoxOffset = FVector(0.0f, 0.0f, 120.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Damage")
	bool bUseSpikeLineDamage = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Damage", meta = (EditCondition = "bUseSpikeLineDamage", ClampMin = "1.0"))
	float SpikeLineLength = 720.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Damage", meta = (EditCondition = "bUseSpikeLineDamage", ClampMin = "1.0"))
	float SpikeLineDepth = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Damage", meta = (EditCondition = "bUseSpikeLineDamage", ClampMin = "1.0"))
	float SpikeLineHeight = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Damage", meta = (EditCondition = "bUseSpikeLineDamage", ClampMin = "1"))
	int32 SpikeLineSampleCount = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Damage", meta = (EditCondition = "bUseSpikeLineDamage", ClampMin = "1.0"))
	float SpikeLineSampleRadius = 135.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Damage", meta = (EditCondition = "bUseSpikeLineDamage"))
	bool bSpikeLineRunsLeftRight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Damage", meta = (EditCondition = "bUseSpikeLineDamage"))
	bool bSpikeLineStartsAtActor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Damage", meta = (EditCondition = "bUseSpikeLineDamage", ClampMin = "0.0"))
	float SpikeLineBackForgiveness = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Damage", meta = (ClampMin = "0.0"))
	float DamageDelay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Damage", meta = (ClampMin = "0.0"))
	float DamageWindowDuration = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Damage", meta = (ClampMin = "0.01"))
	float DamagePulseInterval = 0.04f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Damage", meta = (EditCondition = "bUseSpikeLineDamage"))
	bool bUseSpikeLineTravelTiming = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Damage", meta = (EditCondition = "bUseSpikeLineTravelTiming", ClampMin = "0.0"))
	float SpikeLineDamageTravelTime = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Damage", meta = (EditCondition = "bUseSpikeLineTravelTiming", ClampMin = "0.0"))
	float SpikeLineDamageLingerTime = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Damage", meta = (ClampMin = "0.0"))
	FVector DamageBoundsForgiveness = FVector(80.0f, 70.0f, 70.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Damage")
	bool bDamageEachEnemyOnce = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Lifetime", meta = (ClampMin = "0.1"))
	float IceWallLifetime = 2.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Wall|Debug")
	bool bDebugDamageBox = false;

protected:
	virtual void BeginPlay() override;

private:
	void ActivateIceWallFX();
	void StartDamageWindow();
	void ApplyIceWallDamage();
	void StopDamageWindow();
	void CollectSpikeLineDamageCandidates(TSet<AActor*>& OutCandidateActors, const FCollisionQueryParams& QueryParams) const;
	bool IsActorInsideSpikeLineDamage(const AActor* TargetActor) const;
	bool IsActorBoundsInsideSpikeLine(const FVector& TargetOrigin, const FVector& TargetExtent) const;
	bool IsActorBoundsInsideSpikeLineGeometry(const FVector& TargetOrigin, const FVector& TargetExtent) const;
	bool IsSpikeLineDamageActiveForBounds(const FVector& TargetOrigin, const FVector& TargetExtent) const;
	bool IsActorInsideDamageBox(const AActor* TargetActor, const FVector& BoxCenter, const FQuat& BoxRotation) const;

	UPROPERTY()
	AALSBaseCharacter* Caster = nullptr;

	UPROPERTY()
	TSet<AActor*> DamagedActors;

	float DamageWindowStartTime = -1.0f;

	FTimerHandle DamageTimerHandle;
	FTimerHandle DamagePulseTimerHandle;
	FTimerHandle DamageWindowTimerHandle;
};
