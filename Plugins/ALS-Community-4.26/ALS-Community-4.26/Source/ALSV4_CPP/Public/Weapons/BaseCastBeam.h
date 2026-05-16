#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseCastBeam.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class UParticleSystem;
class USceneComponent;

UENUM(BlueprintType)
enum class EMagicBeamFXLengthAxis : uint8
{
	X UMETA(DisplayName = "Local X"),
	Y UMETA(DisplayName = "Local Y"),
	Z UMETA(DisplayName = "Local Z")
};

UCLASS(Blueprintable)
class ALSV4_CPP_API ABaseCastBeam : public AActor
{
	GENERATED_BODY()

public:
	ABaseCastBeam();

	UFUNCTION(BlueprintCallable, Category = "Magic Beam")
	void InitializeMagicBeam(AActor* InCaster, float OverrideDamage = -1.0f, float OverrideRange = -1.0f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* BeamFXComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Beam|Damage")
	float DamageAmount = 42.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Beam|Damage")
	bool bDamageFirstEnemyOnly = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Beam|Trace", meta = (ClampMin = "1.0"))
	float BeamRange = 2400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Beam|Trace", meta = (ClampMin = "0.0"))
	float BeamRadius = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Beam|Trace")
	bool bStopBeamAtWorldHit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Beam|Trace")
	bool bIgnoreCasterAttachedActors = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Beam|FX")
	UNiagaraSystem* BeamNiagara = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Beam|FX")
	UNiagaraSystem* ImpactNiagara = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Beam|FX")
	UParticleSystem* ImpactParticle = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Beam|FX")
	FVector BeamFXScale = FVector(1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Beam|FX")
	bool bScaleBeamFXLengthToTrace = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Beam|FX", meta = (EditCondition = "bScaleBeamFXLengthToTrace", ClampMin = "1.0"))
	float BeamFXBaseLength = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Beam|FX", meta = (EditCondition = "bScaleBeamFXLengthToTrace"))
	EMagicBeamFXLengthAxis BeamFXLengthAxis = EMagicBeamFXLengthAxis::X;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Beam|FX", meta = (EditCondition = "bScaleBeamFXLengthToTrace"))
	bool bFlipBeamFXLengthAxis = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Beam|FX")
	FVector ImpactFXScale = FVector(1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Beam|FX", meta = (ClampMin = "0.01"))
	float BeamLifetime = 0.28f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Beam|FX")
	FName BeamStartParameterName = TEXT("BeamStart");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Beam|FX")
	FName BeamEndParameterName = TEXT("BeamEnd");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Beam|FX")
	FName BeamLengthParameterName = TEXT("BeamLength");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Beam|Debug")
	bool bDebugBeamTrace = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Beam|Debug", meta = (EditCondition = "bDebugBeamTrace", ClampMin = "0.0"))
	float DebugDrawDuration = 1.0f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent, Category = "Magic Beam")
	void OnBeamHit(AActor* HitActor, const FHitResult& Hit);
	virtual void OnBeamHit_Implementation(AActor* HitActor, const FHitResult& Hit);

private:
	void AddIgnoredActor(AActor* ActorToIgnore);
	bool IsIgnoredActor(const AActor* Actor) const;
	void FireBeam();
	bool TraceWorldHit(const FVector& BeamStart, const FVector& BeamEnd, FHitResult& OutHit) const;
	void DamageEnemiesAlongBeam(const FVector& BeamStart, const FVector& BeamEnd, FVector& InOutBeamEnd);
	void SpawnBeamFX(const FVector& BeamStart, const FVector& BeamEnd);
	void SpawnImpactFX(const FVector& BeamEnd, const FHitResult* Hit) const;

	UPROPERTY()
	AActor* Caster = nullptr;

	UPROPERTY()
	TSet<AActor*> IgnoredActors;

	UPROPERTY()
	TSet<AActor*> DamagedActors;
};
