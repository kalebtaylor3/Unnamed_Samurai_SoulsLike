#include "Weapons/BaseCastBeam.h"

#include "AI/EnemyHealthComponent.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

namespace
{
FRotator MakeBeamFXRotation(const FVector& BeamDirection, EMagicBeamFXLengthAxis LengthAxis, bool bFlipAxis)
{
	const FVector AxisDirection = bFlipAxis ? -BeamDirection : BeamDirection;
	switch (LengthAxis)
	{
		case EMagicBeamFXLengthAxis::Y:
			return FRotationMatrix::MakeFromY(AxisDirection).Rotator();
		case EMagicBeamFXLengthAxis::Z:
			return FRotationMatrix::MakeFromZ(AxisDirection).Rotator();
		case EMagicBeamFXLengthAxis::X:
		default:
			return FRotationMatrix::MakeFromX(AxisDirection).Rotator();
	}
}

FVector ScaleBeamFXLengthAxis(FVector Scale, EMagicBeamFXLengthAxis LengthAxis, float ScaleMultiplier)
{
	switch (LengthAxis)
	{
		case EMagicBeamFXLengthAxis::Y:
			Scale.Y *= ScaleMultiplier;
			break;
		case EMagicBeamFXLengthAxis::Z:
			Scale.Z *= ScaleMultiplier;
			break;
		case EMagicBeamFXLengthAxis::X:
		default:
			Scale.X *= ScaleMultiplier;
			break;
	}

	return Scale;
}
}

ABaseCastBeam::ABaseCastBeam()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	BeamFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BeamFX"));
	BeamFXComponent->SetupAttachment(SceneRoot);
	BeamFXComponent->SetAutoActivate(false);

	InitialLifeSpan = 0.6f;
}

void ABaseCastBeam::BeginPlay()
{
	Super::BeginPlay();

	if (!Caster)
	{
		Caster = GetOwner() ? GetOwner() : GetInstigator();
	}

	AddIgnoredActor(Caster);
	AddIgnoredActor(GetOwner());
	AddIgnoredActor(GetInstigator());

	FireBeam();
	SetLifeSpan(FMath::Max(BeamLifetime, 0.05f) + 0.1f);
}

void ABaseCastBeam::InitializeMagicBeam(AActor* InCaster, float OverrideDamage, float OverrideRange)
{
	Caster = InCaster ? InCaster : Caster;

	if (OverrideDamage >= 0.0f)
	{
		DamageAmount = OverrideDamage;
	}

	if (OverrideRange > 0.0f)
	{
		BeamRange = OverrideRange;
	}

	AddIgnoredActor(Caster);
	AddIgnoredActor(GetOwner());
	AddIgnoredActor(GetInstigator());
	DamagedActors.Empty();
}

void ABaseCastBeam::OnBeamHit_Implementation(AActor* HitActor, const FHitResult& Hit)
{
}

void ABaseCastBeam::AddIgnoredActor(AActor* ActorToIgnore)
{
	if (!ActorToIgnore || ActorToIgnore == this || IgnoredActors.Contains(ActorToIgnore))
	{
		return;
	}

	IgnoredActors.Add(ActorToIgnore);

	if (!bIgnoreCasterAttachedActors)
	{
		return;
	}

	TArray<AActor*> AttachedActors;
	ActorToIgnore->GetAttachedActors(AttachedActors);
	for (AActor* AttachedActor : AttachedActors)
	{
		AddIgnoredActor(AttachedActor);
	}
}

bool ABaseCastBeam::IsIgnoredActor(const AActor* Actor) const
{
	if (!Actor || Actor == this || IgnoredActors.Contains(Actor))
	{
		return true;
	}

	const AActor* ParentActor = Actor->GetAttachParentActor();
	if (ParentActor && IgnoredActors.Contains(ParentActor))
	{
		return true;
	}

	const AActor* OwnerActor = Actor->GetOwner();
	for (int32 Depth = 0; OwnerActor && Depth < 8; ++Depth)
	{
		if (IgnoredActors.Contains(OwnerActor))
		{
			return true;
		}

		OwnerActor = OwnerActor->GetOwner();
	}

	return false;
}

void ABaseCastBeam::FireBeam()
{
	if (!GetWorld())
	{
		return;
	}

	const FVector BeamStart = GetActorLocation();
	const FVector BeamDirection = GetActorForwardVector().GetSafeNormal();
	FVector BeamEnd = BeamStart + BeamDirection * BeamRange;

	FHitResult WorldHit;
	const bool bHitWorld = bStopBeamAtWorldHit && TraceWorldHit(BeamStart, BeamEnd, WorldHit);
	if (bHitWorld)
	{
		BeamEnd = WorldHit.ImpactPoint;
	}

	DamageEnemiesAlongBeam(BeamStart, BeamEnd, BeamEnd);
	SpawnBeamFX(BeamStart, BeamEnd);
	SpawnImpactFX(BeamEnd, bHitWorld ? &WorldHit : nullptr);

	if (bDebugBeamTrace)
	{
		DrawDebugLine(GetWorld(), BeamStart, BeamEnd, FColor::Yellow, false, DebugDrawDuration, 0, 3.0f);
		DrawDebugSphere(GetWorld(), BeamEnd, BeamRadius + 8.0f, 16, FColor::Orange, false, DebugDrawDuration);
	}
}

bool ABaseCastBeam::TraceWorldHit(const FVector& BeamStart, const FVector& BeamEnd, FHitResult& OutHit) const
{
	FCollisionQueryParams QueryParams(FName(TEXT("MagicBeamWorldTrace")), false, this);
	for (AActor* Ignored : IgnoredActors)
	{
		if (Ignored)
		{
			QueryParams.AddIgnoredActor(Ignored);
		}
	}

	return GetWorld()->LineTraceSingleByObjectType(
		OutHit,
		BeamStart,
		BeamEnd,
		FCollisionObjectQueryParams(ECC_WorldStatic),
		QueryParams);
}

void ABaseCastBeam::DamageEnemiesAlongBeam(const FVector& BeamStart, const FVector& BeamEnd, FVector& InOutBeamEnd)
{
	if (!GetWorld())
	{
		return;
	}

	FCollisionQueryParams QueryParams(FName(TEXT("MagicBeamEnemyTrace")), false, this);
	for (AActor* Ignored : IgnoredActors)
	{
		if (Ignored)
		{
			QueryParams.AddIgnoredActor(Ignored);
		}
	}

	TArray<FHitResult> Hits;
	const FCollisionShape BeamShape = FCollisionShape::MakeSphere(FMath::Max(0.0f, BeamRadius));
	const bool bHitAnything = BeamRadius > 0.0f
		? GetWorld()->SweepMultiByObjectType(
			Hits,
			BeamStart,
			BeamEnd,
			FQuat::Identity,
			FCollisionObjectQueryParams(ECC_Pawn),
			BeamShape,
			QueryParams)
		: GetWorld()->LineTraceMultiByObjectType(
			Hits,
			BeamStart,
			BeamEnd,
			FCollisionObjectQueryParams(ECC_Pawn),
			QueryParams);

	if (!bHitAnything)
	{
		return;
	}

	Hits.Sort([](const FHitResult& Left, const FHitResult& Right)
	{
		return Left.Distance < Right.Distance;
	});

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || IsIgnoredActor(HitActor) || DamagedActors.Contains(HitActor))
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
		OnBeamHit(HitActor, Hit);

		if (bDamageFirstEnemyOnly)
		{
			InOutBeamEnd = Hit.ImpactPoint.IsNearlyZero() ? Hit.Location : Hit.ImpactPoint;
			return;
		}
	}
}

void ABaseCastBeam::SpawnBeamFX(const FVector& BeamStart, const FVector& BeamEnd)
{
	if (!BeamFXComponent)
	{
		return;
	}

	if (BeamNiagara)
	{
		BeamFXComponent->SetAsset(BeamNiagara);
	}

	const float BeamLength = FVector::Dist(BeamStart, BeamEnd);
	const FVector BeamDirection = (BeamEnd - BeamStart).GetSafeNormal();
	FVector FinalBeamScale = BeamFXScale;
	if (bScaleBeamFXLengthToTrace && BeamFXBaseLength > 0.0f)
	{
		FinalBeamScale = ScaleBeamFXLengthAxis(FinalBeamScale, BeamFXLengthAxis, BeamLength / BeamFXBaseLength);
	}

	BeamFXComponent->SetWorldLocationAndRotation(BeamStart, MakeBeamFXRotation(BeamDirection, BeamFXLengthAxis, bFlipBeamFXLengthAxis));
	BeamFXComponent->SetWorldScale3D(FinalBeamScale);
	BeamFXComponent->SetVariableVec3(BeamStartParameterName, BeamStart);
	BeamFXComponent->SetVariableVec3(BeamEndParameterName, BeamEnd);
	BeamFXComponent->SetVariableFloat(BeamLengthParameterName, BeamLength);
	BeamFXComponent->Activate(true);
}

void ABaseCastBeam::SpawnImpactFX(const FVector& BeamEnd, const FHitResult* Hit) const
{
	const FVector ImpactNormal = Hit && !Hit->ImpactNormal.IsNearlyZero()
		? FVector(Hit->ImpactNormal).GetSafeNormal()
		: -GetActorForwardVector();
	const FRotator ImpactRotation = ImpactNormal.Rotation();

	if (ImpactNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ImpactNiagara,
			BeamEnd,
			ImpactRotation,
			ImpactFXScale);
	}

	if (ImpactParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ImpactParticle,
			BeamEnd,
			ImpactRotation,
			ImpactFXScale);
	}
}
