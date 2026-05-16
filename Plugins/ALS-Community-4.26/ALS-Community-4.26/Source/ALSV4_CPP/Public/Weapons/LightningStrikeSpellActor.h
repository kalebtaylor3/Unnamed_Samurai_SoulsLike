#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LightningStrikeSpellActor.generated.h"

class AALSBaseCharacter;
class UNiagaraSystem;
class UParticleSystem;
class USceneComponent;

UCLASS(Blueprintable)
class ALSV4_CPP_API ALightningStrikeSpellActor : public AActor
{
	GENERATED_BODY()

public:
	ALightningStrikeSpellActor();

	UFUNCTION(BlueprintCallable, Category = "Lightning Strike")
	void InitializeLightningStrike(AALSBaseCharacter* InCaster, AActor* InLockedTarget, const FVector& InStormCenter);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|FX")
	UNiagaraSystem* StrikeNiagara = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|FX")
	UParticleSystem* StrikeParticle = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Impact")
	UNiagaraSystem* ImpactNiagara = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Impact")
	UParticleSystem* ImpactParticle = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|FX")
	UNiagaraSystem* CloudNiagara = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|FX")
	FVector StrikeFXScale = FVector(1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Impact")
	FVector ImpactFXScale = FVector(1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Impact")
	float ImpactFXGroundOffset = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|FX", meta = (ClampMin = "0.0"))
	float LightningScaleVertical = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|FX")
	FName LightningScaleVerticalParameterName = TEXT("Lightning Scale Vertical");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|FX")
	FVector CloudFXScale = FVector(1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|FX")
	float CloudHeight = 650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|FX")
	float StrikeFXGroundOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Pattern", meta = (ClampMin = "1"))
	int32 StrikeCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Pattern", meta = (ClampMin = "0.0"))
	float FirstStrikeDelay = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Pattern", meta = (ClampMin = "0.0"))
	float StrikeInterval = 0.14f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Pattern", meta = (ClampMin = "0.0"))
	float UnlockedStrikeRadius = 360.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Pattern", meta = (ClampMin = "0.0"))
	float LockedStrikeRadius = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Pattern")
	bool bGuaranteeFirstStrikeOnLockedTarget = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Pattern")
	bool bUseCloudFormationPattern = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Pattern", meta = (EditCondition = "bUseCloudFormationPattern", ClampMin = "0.0"))
	float FormationJitterRadius = 55.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Pattern", meta = (EditCondition = "bUseCloudFormationPattern", ClampMin = "0.0"))
	float FormationForwardBias = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Damage")
	float DamageAmount = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Damage", meta = (ClampMin = "1.0"))
	float DamageRadius = 190.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Damage")
	bool bDamageEachEnemyOncePerStrike = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Damage")
	bool bDamageEachEnemyOncePerSpell = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Ground")
	float GroundTraceHeight = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Ground")
	float GroundTraceDepth = 2200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Ground")
	float GroundOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Lifetime", meta = (ClampMin = "0.1"))
	float SpellLifetime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Debug")
	bool bDebugStrikes = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Strike|Debug", meta = (EditCondition = "bDebugStrikes", ClampMin = "0.0"))
	float DebugDrawDuration = 1.0f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void SpawnCloudFX();
	void ScheduleStrikes();
	void ResolveStrike(int32 StrikeIndex);
	FVector GetStrikeLocation(int32 StrikeIndex) const;
	FVector GetCloudFormationOffset(int32 StrikeIndex, float Radius) const;
	FVector GetRandomJitter(float Radius) const;
	FVector FindGroundLocation(const FVector& DesiredLocation) const;
	bool ApplyStrikeDamage(const FVector& StrikeLocation);
	void SpawnImpactFX(const FVector& StrikeLocation) const;

	UPROPERTY()
	AALSBaseCharacter* Caster = nullptr;

	UPROPERTY()
	AActor* LockedTarget = nullptr;

	UPROPERTY()
	TSet<AActor*> DamagedActors;

	FVector StormCenter = FVector::ZeroVector;
	TArray<FTimerHandle> StrikeTimerHandles;
};
