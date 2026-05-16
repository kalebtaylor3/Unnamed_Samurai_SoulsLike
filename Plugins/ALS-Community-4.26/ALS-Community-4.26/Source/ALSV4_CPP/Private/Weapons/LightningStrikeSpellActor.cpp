#include "Weapons/LightningStrikeSpellActor.h"

#include "AI/EnemyHealthComponent.h"
#include "Character/ALSBaseCharacter.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"

ALightningStrikeSpellActor::ALightningStrikeSpellActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	InitialLifeSpan = 2.0f;
}

void ALightningStrikeSpellActor::BeginPlay()
{
	Super::BeginPlay();

	if (StormCenter.IsNearlyZero())
	{
		StormCenter = GetActorLocation();
	}

	SpawnCloudFX();
	ScheduleStrikes();
	SetLifeSpan(FMath::Max(SpellLifetime, FirstStrikeDelay + StrikeInterval * FMath::Max(StrikeCount - 1, 0) + 0.5f));
}

void ALightningStrikeSpellActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		for (FTimerHandle& StrikeTimerHandle : StrikeTimerHandles)
		{
			GetWorldTimerManager().ClearTimer(StrikeTimerHandle);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ALightningStrikeSpellActor::InitializeLightningStrike(AALSBaseCharacter* InCaster, AActor* InLockedTarget, const FVector& InStormCenter)
{
	Caster = InCaster;
	LockedTarget = InLockedTarget;
	StormCenter = InStormCenter;
	SetOwner(InCaster);
	SetInstigator(InCaster);
}

void ALightningStrikeSpellActor::SpawnCloudFX()
{
	if (!CloudNiagara || !GetWorld())
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		CloudNiagara,
		StormCenter + FVector(0.0f, 0.0f, CloudHeight),
		GetActorRotation(),
		CloudFXScale);
}

void ALightningStrikeSpellActor::ScheduleStrikes()
{
	if (!GetWorld())
	{
		return;
	}

	const int32 SafeStrikeCount = FMath::Max(1, StrikeCount);
	StrikeTimerHandles.SetNum(SafeStrikeCount);
	for (int32 Index = 0; Index < SafeStrikeCount; ++Index)
	{
		const float Delay = FirstStrikeDelay + StrikeInterval * Index;
		GetWorldTimerManager().SetTimer(
			StrikeTimerHandles[Index],
			FTimerDelegate::CreateUObject(this, &ALightningStrikeSpellActor::ResolveStrike, Index),
			Delay,
			false);
	}
}

void ALightningStrikeSpellActor::ResolveStrike(int32 StrikeIndex)
{
	if (!GetWorld())
	{
		return;
	}

	const FVector StrikeLocation = FindGroundLocation(GetStrikeLocation(StrikeIndex));

	const FVector StrikeFXLocation = StrikeLocation + FVector(0.0f, 0.0f, StrikeFXGroundOffset);

	if (StrikeNiagara)
	{
		UNiagaraComponent* StrikeFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			StrikeNiagara,
			StrikeFXLocation,
			GetActorRotation(),
			StrikeFXScale);

		if (StrikeFX && !LightningScaleVerticalParameterName.IsNone())
		{
			const FString UserParameterName = FString::Printf(TEXT("User.%s"), *LightningScaleVerticalParameterName.ToString());
			StrikeFX->SetNiagaraVariableFloat(UserParameterName, LightningScaleVertical);
			StrikeFX->SetVariableFloat(LightningScaleVerticalParameterName, LightningScaleVertical);
		}
	}

	if (StrikeParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			StrikeParticle,
			StrikeFXLocation,
			GetActorRotation(),
			StrikeFXScale);
	}

	const bool bDamagedEnemy = ApplyStrikeDamage(StrikeLocation);
	if (bDamagedEnemy)
	{
		SpawnImpactFX(StrikeLocation);
	}

	if (bDebugStrikes)
	{
		DrawDebugSphere(GetWorld(), StrikeLocation, DamageRadius, 24, FColor::Yellow, false, DebugDrawDuration, 0, 2.0f);
		DrawDebugLine(GetWorld(), StrikeLocation + FVector(0.0f, 0.0f, CloudHeight), StrikeLocation, FColor::Cyan, false, DebugDrawDuration, 0, 2.0f);
	}
}

FVector ALightningStrikeSpellActor::GetStrikeLocation(int32 StrikeIndex) const
{
	const bool bHasLockedTarget = LockedTarget != nullptr;
	const float Radius = bHasLockedTarget ? LockedStrikeRadius : UnlockedStrikeRadius;

	if (bHasLockedTarget && bGuaranteeFirstStrikeOnLockedTarget && StrikeIndex == 0)
	{
		return StormCenter;
	}

	if (bUseCloudFormationPattern)
	{
		return StormCenter + GetCloudFormationOffset(StrikeIndex, Radius) + GetRandomJitter(FormationJitterRadius);
	}

	const float Angle = FMath::DegreesToRadians(FMath::FRandRange(0.0f, 360.0f));
	const float Distance = FMath::Sqrt(FMath::FRand()) * Radius;
	const FVector Offset = FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.0f);
	return StormCenter + Offset;
}

FVector ALightningStrikeSpellActor::GetCloudFormationOffset(int32 StrikeIndex, float Radius) const
{
	const FVector Forward = GetActorForwardVector().GetSafeNormal2D();
	const FVector Right = GetActorRightVector().GetSafeNormal2D();
	const FVector BiasedCenter = Forward * Radius * FormationForwardBias;

	static const FVector2D PatternOffsets[] =
	{
		FVector2D(0.0f, 0.0f),
		FVector2D(-0.55f, 0.18f),
		FVector2D(0.55f, 0.16f),
		FVector2D(-0.28f, 0.68f),
		FVector2D(0.32f, 0.72f),
		FVector2D(0.0f, -0.42f),
		FVector2D(-0.78f, 0.55f),
		FVector2D(0.78f, 0.52f)
	};

	const int32 PatternIndex = FMath::Abs(StrikeIndex) % UE_ARRAY_COUNT(PatternOffsets);
	const FVector2D PatternOffset = PatternOffsets[PatternIndex];
	return BiasedCenter + Right * PatternOffset.X * Radius + Forward * PatternOffset.Y * Radius;
}

FVector ALightningStrikeSpellActor::GetRandomJitter(float Radius) const
{
	if (Radius <= 0.0f)
	{
		return FVector::ZeroVector;
	}

	const float Angle = FMath::DegreesToRadians(FMath::FRandRange(0.0f, 360.0f));
	const float Distance = FMath::Sqrt(FMath::FRand()) * Radius;
	return FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.0f);
}

FVector ALightningStrikeSpellActor::FindGroundLocation(const FVector& DesiredLocation) const
{
	if (!GetWorld())
	{
		return DesiredLocation;
	}

	const FVector TraceStart = DesiredLocation + FVector(0.0f, 0.0f, GroundTraceHeight);
	const FVector TraceEnd = DesiredLocation - FVector(0.0f, 0.0f, GroundTraceDepth);

	FHitResult Hit;
	FCollisionQueryParams QueryParams(FName(TEXT("LightningStrikeActorGroundTrace")), false, this);
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(Caster);

	const bool bHitGround = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
	return bHitGround ? FVector(Hit.ImpactPoint) + FVector(0.0f, 0.0f, GroundOffset) : DesiredLocation;
}

bool ALightningStrikeSpellActor::ApplyStrikeDamage(const FVector& StrikeLocation)
{
	if (!GetWorld())
	{
		return false;
	}

	FCollisionQueryParams QueryParams(FName(TEXT("LightningStrikeDamage")), false, this);
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(Caster);

	TArray<FOverlapResult> Overlaps;
	const bool bHitAnything = GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		StrikeLocation,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(DamageRadius),
		QueryParams);

	if (!bHitAnything)
	{
		return false;
	}

	bool bDamagedAnyEnemy = false;
	TSet<AActor*> DamagedActorsThisStrike;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || HitActor == Caster)
		{
			continue;
		}

		if (bDamageEachEnemyOncePerStrike && DamagedActorsThisStrike.Contains(HitActor))
		{
			continue;
		}

		if (bDamageEachEnemyOncePerSpell && DamagedActors.Contains(HitActor))
		{
			continue;
		}

		UEnemyHealthComponent* HealthComponent = HitActor->FindComponentByClass<UEnemyHealthComponent>();
		if (!HealthComponent || HealthComponent->IsDeadOrOutOfHealth())
		{
			continue;
		}

		HealthComponent->TakeDamage(DamageAmount);
		DamagedActorsThisStrike.Add(HitActor);
		DamagedActors.Add(HitActor);
		bDamagedAnyEnemy = true;
	}

	return bDamagedAnyEnemy;
}

void ALightningStrikeSpellActor::SpawnImpactFX(const FVector& StrikeLocation) const
{
	if (!GetWorld())
	{
		return;
	}

	const FVector ImpactLocation = StrikeLocation + FVector(0.0f, 0.0f, ImpactFXGroundOffset);
	const FRotator ImpactRotation = GetActorRotation();

	if (ImpactNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ImpactNiagara,
			ImpactLocation,
			ImpactRotation,
			ImpactFXScale);
	}

	if (ImpactParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ImpactParticle,
			ImpactLocation,
			ImpactRotation,
			ImpactFXScale);
	}
}
