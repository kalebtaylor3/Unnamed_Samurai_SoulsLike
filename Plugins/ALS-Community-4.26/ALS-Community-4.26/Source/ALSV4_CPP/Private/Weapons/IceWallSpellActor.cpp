#include "Weapons/IceWallSpellActor.h"

#include "AI/EnemyHealthComponent.h"
#include "Character/ALSBaseCharacter.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"
#include "UObject/UObjectIterator.h"

AIceWallSpellActor::AIceWallSpellActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	IceWallFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("IceWallFX"));
	IceWallFX->SetupAttachment(SceneRoot);
	IceWallFX->SetAutoActivate(false);
}

void AIceWallSpellActor::BeginPlay()
{
	Super::BeginPlay();

	ActivateIceWallFX();

	if (DamageDelay <= 0.0f)
	{
		StartDamageWindow();
	}
	else if (GetWorld())
	{
		GetWorldTimerManager().SetTimer(
			DamageTimerHandle,
			this,
			&AIceWallSpellActor::StartDamageWindow,
			DamageDelay,
			false);
	}

	SetLifeSpan(IceWallLifetime);
}

void AIceWallSpellActor::InitializeIceWall(AALSBaseCharacter* InCaster)
{
	Caster = InCaster;
	SetOwner(InCaster);
	SetInstigator(InCaster);
}

void AIceWallSpellActor::ActivateIceWallFX()
{
	if (!IceWallFX)
	{
		return;
	}

	if (IceWallNiagara)
	{
		IceWallFX->SetAsset(IceWallNiagara);
	}

	IceWallFX->Activate(true);
}

void AIceWallSpellActor::StartDamageWindow()
{
	DamageWindowStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	ApplyIceWallDamage();

	if (!GetWorld() || DamageWindowDuration <= 0.0f)
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		DamagePulseTimerHandle,
		this,
		&AIceWallSpellActor::ApplyIceWallDamage,
		DamagePulseInterval,
		true);

	GetWorldTimerManager().SetTimer(
		DamageWindowTimerHandle,
		this,
		&AIceWallSpellActor::StopDamageWindow,
		DamageWindowDuration,
		false);
}

void AIceWallSpellActor::ApplyIceWallDamage()
{
	if (!GetWorld())
	{
		return;
	}

	const FVector BoxCenter = GetActorLocation() + GetActorTransform().TransformVectorNoScale(DamageBoxOffset);
	const FQuat BoxRotation = GetActorQuat();
	const FCollisionShape DamageShape = FCollisionShape::MakeBox(DamageBoxHalfExtent + DamageBoundsForgiveness);

	FCollisionQueryParams QueryParams(FName(TEXT("IceWallDamage")), false, this);
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(Caster);

	if (Caster)
	{
		TArray<AActor*> AttachedActors;
		Caster->GetAttachedActors(AttachedActors);
		for (AActor* AttachedActor : AttachedActors)
		{
			if (AttachedActor)
			{
				QueryParams.AddIgnoredActor(AttachedActor);
			}
		}
	}

	TSet<AActor*> CandidateActors;
	if (bUseSpikeLineDamage)
	{
		CollectSpikeLineDamageCandidates(CandidateActors, QueryParams);
	}
	else
	{
		TArray<FOverlapResult> Overlaps;
		const bool bHitAnything = GetWorld()->OverlapMultiByObjectType(
			Overlaps,
			BoxCenter,
			BoxRotation,
			FCollisionObjectQueryParams(ECC_Pawn),
			DamageShape,
			QueryParams);

		if (bDebugDamageBox)
		{
			DrawDebugBox(GetWorld(), BoxCenter, DamageBoxHalfExtent, BoxRotation, FColor::Blue, false, 1.25f, 0, 2.0f);
			DrawDebugBox(GetWorld(), BoxCenter, DamageBoxHalfExtent + DamageBoundsForgiveness, BoxRotation, bHitAnything ? FColor::Cyan : FColor::Silver, false, 1.25f, 0, 1.0f);
		}

		if (bHitAnything)
		{
			for (const FOverlapResult& Overlap : Overlaps)
			{
				if (AActor* HitActor = Overlap.GetActor())
				{
					CandidateActors.Add(HitActor);
				}
			}
		}
	}

	for (TObjectIterator<UEnemyHealthComponent> It; It; ++It)
	{
		UEnemyHealthComponent* HealthComponent = *It;
		if (!HealthComponent || HealthComponent->GetWorld() != GetWorld())
		{
			continue;
		}

		AActor* OwnerActor = HealthComponent->GetOwner();
		const bool bInsideDamage = bUseSpikeLineDamage
			? IsActorInsideSpikeLineDamage(OwnerActor)
			: IsActorInsideDamageBox(OwnerActor, BoxCenter, BoxRotation);

		if (OwnerActor && bInsideDamage)
		{
			CandidateActors.Add(OwnerActor);
		}
	}

	for (AActor* HitActor : CandidateActors)
	{
		if (!HitActor || HitActor == Caster)
		{
			continue;
		}

		if (bDamageEachEnemyOnce && DamagedActors.Contains(HitActor))
		{
			continue;
		}

		UEnemyHealthComponent* HealthComponent = HitActor->FindComponentByClass<UEnemyHealthComponent>();
		if (!HealthComponent || HealthComponent->IsDeadOrOutOfHealth())
		{
			continue;
		}

		HealthComponent->TakeDamage(DamageAmount);
		DamagedActors.Add(HitActor);
	}
}

void AIceWallSpellActor::StopDamageWindow()
{
	if (!GetWorld())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(DamagePulseTimerHandle);
}

void AIceWallSpellActor::CollectSpikeLineDamageCandidates(TSet<AActor*>& OutCandidateActors, const FCollisionQueryParams& QueryParams) const
{
	if (!GetWorld())
	{
		return;
	}

	const int32 SampleCount = FMath::Max(1, SpikeLineSampleCount);
	const FVector Center = GetActorLocation() + GetActorTransform().TransformVectorNoScale(DamageBoxOffset);
	const FVector LineAxis = bSpikeLineRunsLeftRight ? GetActorRightVector() : GetActorForwardVector();
	const float Step = SampleCount > 1 ? SpikeLineLength / static_cast<float>(SampleCount - 1) : 0.0f;
	const float StartOffset = bSpikeLineStartsAtActor || SampleCount <= 1 ? 0.0f : -SpikeLineLength * 0.5f;
	const float Radius = FMath::Max(1.0f, SpikeLineSampleRadius);

	bool bAnyHit = false;
	for (TObjectIterator<UEnemyHealthComponent> It; It; ++It)
	{
		UEnemyHealthComponent* HealthComponent = *It;
		if (!HealthComponent || HealthComponent->GetWorld() != GetWorld())
		{
			continue;
		}

		AActor* OwnerActor = HealthComponent->GetOwner();
		if (!OwnerActor || OwnerActor == Caster || HealthComponent->IsDeadOrOutOfHealth())
		{
			continue;
		}

		FVector TargetOrigin = FVector::ZeroVector;
		FVector TargetExtent = FVector::ZeroVector;
		OwnerActor->GetActorBounds(false, TargetOrigin, TargetExtent);

		if (IsActorBoundsInsideSpikeLine(TargetOrigin, TargetExtent))
		{
			OutCandidateActors.Add(OwnerActor);
			bAnyHit = true;
		}
	}

	if (bDebugDamageBox)
	{
		const FVector DepthAxis = bSpikeLineRunsLeftRight ? GetActorForwardVector() : GetActorRightVector();
		const FVector HeightAxis = FVector::UpVector;
		const FVector LineStart = Center + LineAxis * StartOffset;
		const FVector LineEnd = Center + LineAxis * (StartOffset + Step * (SampleCount - 1));
		const float Elapsed = DamageWindowStartTime >= 0.0f && GetWorld()
			? FMath::Max(0.0f, GetWorld()->GetTimeSeconds() - DamageWindowStartTime)
			: 0.0f;
		const float ActiveAlpha = SpikeLineDamageTravelTime > 0.0f
			? FMath::Clamp(Elapsed / SpikeLineDamageTravelTime, 0.0f, 1.0f)
			: 1.0f;
		const FVector ActiveEnd = Center + LineAxis * FMath::Lerp(StartOffset, StartOffset + Step * (SampleCount - 1), ActiveAlpha);
		for (int32 Index = 0; Index < SampleCount; ++Index)
		{
			const FVector SampleLocation = Center + LineAxis * (StartOffset + Step * Index);
			DrawDebugSphere(GetWorld(), SampleLocation, Radius, 16, bAnyHit ? FColor::Cyan : FColor::Blue, false, 1.25f, 0, 1.0f);
		}
		DrawDebugLine(GetWorld(), LineStart, LineEnd, bAnyHit ? FColor::Cyan : FColor::Silver, false, 1.25f, 0, 2.0f);
		DrawDebugLine(GetWorld(), LineStart, ActiveEnd, FColor::Red, false, 1.25f, 0, 4.0f);
		DrawDebugLine(GetWorld(), Center - DepthAxis * SpikeLineDepth * 0.5f, Center + DepthAxis * SpikeLineDepth * 0.5f, FColor::Yellow, false, 1.25f, 0, 1.0f);
		DrawDebugLine(GetWorld(), Center - HeightAxis * SpikeLineHeight * 0.5f, Center + HeightAxis * SpikeLineHeight * 0.5f, FColor::Green, false, 1.25f, 0, 1.0f);
	}
}

bool AIceWallSpellActor::IsActorInsideSpikeLineDamage(const AActor* TargetActor) const
{
	if (!TargetActor)
	{
		return false;
	}

	FVector TargetOrigin = FVector::ZeroVector;
	FVector TargetExtent = FVector::ZeroVector;
	TargetActor->GetActorBounds(false, TargetOrigin, TargetExtent);

	return IsActorBoundsInsideSpikeLine(TargetOrigin, TargetExtent);
}

bool AIceWallSpellActor::IsActorBoundsInsideSpikeLine(const FVector& TargetOrigin, const FVector& TargetExtent) const
{
	return IsActorBoundsInsideSpikeLineGeometry(TargetOrigin, TargetExtent)
		&& IsSpikeLineDamageActiveForBounds(TargetOrigin, TargetExtent);
}

bool AIceWallSpellActor::IsActorBoundsInsideSpikeLineGeometry(const FVector& TargetOrigin, const FVector& TargetExtent) const
{
	const FVector Center = GetActorLocation() + GetActorTransform().TransformVectorNoScale(DamageBoxOffset);
	const FVector LocalPosition = GetActorQuat().UnrotateVector(TargetOrigin - Center);

	const float Along = bSpikeLineRunsLeftRight ? LocalPosition.Y : LocalPosition.X;
	const float Depth = bSpikeLineRunsLeftRight ? LocalPosition.X : LocalPosition.Y;
	const float Vertical = LocalPosition.Z;

	const float TargetAlongExtent = bSpikeLineRunsLeftRight ? TargetExtent.Y : TargetExtent.X;
	const float TargetDepthExtent = bSpikeLineRunsLeftRight ? TargetExtent.X : TargetExtent.Y;

	const float SegmentMin = bSpikeLineStartsAtActor ? -SpikeLineBackForgiveness : -SpikeLineLength * 0.5f;
	const float SegmentMax = bSpikeLineStartsAtActor ? SpikeLineLength : SpikeLineLength * 0.5f;
	const float ActorMinAlong = Along - TargetAlongExtent;
	const float ActorMaxAlong = Along + TargetAlongExtent;

	const bool bOverlapsLength = ActorMaxAlong >= SegmentMin && ActorMinAlong <= SegmentMax;
	const bool bOverlapsDepth = FMath::Abs(Depth) <= SpikeLineDepth * 0.5f + DamageBoundsForgiveness.Y + TargetDepthExtent;
	const bool bOverlapsHeight = FMath::Abs(Vertical) <= SpikeLineHeight * 0.5f + DamageBoundsForgiveness.Z + TargetExtent.Z;

	return bOverlapsLength && bOverlapsDepth && bOverlapsHeight;
}

bool AIceWallSpellActor::IsSpikeLineDamageActiveForBounds(const FVector& TargetOrigin, const FVector& TargetExtent) const
{
	if (!bUseSpikeLineTravelTiming || !GetWorld() || DamageWindowStartTime < 0.0f || SpikeLineDamageTravelTime <= 0.0f)
	{
		return true;
	}

	const FVector Center = GetActorLocation() + GetActorTransform().TransformVectorNoScale(DamageBoxOffset);
	const FVector LocalPosition = GetActorQuat().UnrotateVector(TargetOrigin - Center);

	const float Along = bSpikeLineRunsLeftRight ? LocalPosition.Y : LocalPosition.X;
	const float TargetAlongExtent = bSpikeLineRunsLeftRight ? TargetExtent.Y : TargetExtent.X;
	const float SegmentMin = bSpikeLineStartsAtActor ? -SpikeLineBackForgiveness : -SpikeLineLength * 0.5f;
	const float SegmentMax = bSpikeLineStartsAtActor ? SpikeLineLength : SpikeLineLength * 0.5f;
	const float TravelLength = FMath::Max(1.0f, SegmentMax - SegmentMin);
	const float ActorEntryAlong = FMath::Clamp(Along - TargetAlongExtent, SegmentMin, SegmentMax);
	const float ActorExitAlong = FMath::Clamp(Along + TargetAlongExtent, SegmentMin, SegmentMax);

	const float EntryAlpha = FMath::Clamp((ActorEntryAlong - SegmentMin) / TravelLength, 0.0f, 1.0f);
	const float ExitAlpha = FMath::Clamp((ActorExitAlong - SegmentMin) / TravelLength, 0.0f, 1.0f);
	const float ActiveStartTime = EntryAlpha * SpikeLineDamageTravelTime;
	const float ActiveEndTime = ExitAlpha * SpikeLineDamageTravelTime + SpikeLineDamageLingerTime;
	const float Elapsed = FMath::Max(0.0f, GetWorld()->GetTimeSeconds() - DamageWindowStartTime);

	return Elapsed >= ActiveStartTime && Elapsed <= ActiveEndTime;
}

bool AIceWallSpellActor::IsActorInsideDamageBox(const AActor* TargetActor, const FVector& BoxCenter, const FQuat& BoxRotation) const
{
	if (!TargetActor)
	{
		return false;
	}

	FVector TargetOrigin = FVector::ZeroVector;
	FVector TargetExtent = FVector::ZeroVector;
	TargetActor->GetActorBounds(false, TargetOrigin, TargetExtent);

	const FVector LocalPosition = BoxRotation.UnrotateVector(TargetOrigin - BoxCenter);
	const FVector AllowedExtent = DamageBoxHalfExtent + DamageBoundsForgiveness + TargetExtent;

	return FMath::Abs(LocalPosition.X) <= AllowedExtent.X
		&& FMath::Abs(LocalPosition.Y) <= AllowedExtent.Y
		&& FMath::Abs(LocalPosition.Z) <= AllowedExtent.Z;
}
