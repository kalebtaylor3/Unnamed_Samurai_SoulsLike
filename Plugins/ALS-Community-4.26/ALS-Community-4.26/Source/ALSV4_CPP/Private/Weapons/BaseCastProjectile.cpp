#include "Weapons/BaseCastProjectile.h"

#include "AI/EnemyHealthComponent.h"
#include "Character/ALSBaseCharacter.h"
#include "Character/ALSPlayerCameraManager.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "UObject/UObjectIterator.h"

ABaseCastProjectile::ABaseCastProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	RootComponent = Collision;
	Collision->InitSphereRadius(12.0f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Collision->SetCollisionObjectType(ECC_WorldDynamic);
	Collision->SetCollisionResponseToAllChannels(ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	Collision->SetGenerateOverlapEvents(true);
	Collision->SetNotifyRigidBodyCollision(true);

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(Collision);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TrailFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailFX"));
	TrailFX->SetupAttachment(Collision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = InitialSpeed;
	ProjectileMovement->MaxSpeed = MaxSpeed;
	ProjectileMovement->bRotationFollowsVelocity = bRotationFollowsVelocity;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = ProjectileGravityScale;

	InitialLifeSpan = 8.0f;
}

void ABaseCastProjectile::BeginPlay()
{
	Super::BeginPlay();

	Collision->OnComponentBeginOverlap.AddDynamic(this, &ABaseCastProjectile::OnProjectileOverlap);
	Collision->OnComponentHit.AddDynamic(this, &ABaseCastProjectile::OnProjectileHit);

	if (!Caster)
	{
		Caster = GetOwner() ? GetOwner() : GetInstigator();
	}

	AddIgnoredActor(Caster);
	AddIgnoredActor(GetOwner());
	AddIgnoredActor(GetInstigator());

	RefreshHomingTarget();
	ResetMagicLaunchState();
	PreviousLocation = GetActorLocation();
}

void ABaseCastProjectile::InitializeMagicProjectile(AActor* InCaster, AActor* InTarget, float OverrideDamage, float OverrideSpeed)
{
	Caster = InCaster ? InCaster : Caster;
	HomingTarget = InTarget ? InTarget : HomingTarget;

	if (OverrideDamage >= 0.0f)
	{
		DamageAmount = OverrideDamage;
	}

	if (OverrideSpeed > 0.0f)
	{
		InitialSpeed = OverrideSpeed;
		MaxSpeed = FMath::Max(MaxSpeed, OverrideSpeed);
	}

	AddIgnoredActor(Caster);
	AddIgnoredActor(GetOwner());
	AddIgnoredActor(GetInstigator());

	DamagedActors.Empty();
	bHasImpacted = false;

	RefreshHomingTarget();
	ResetMagicLaunchState();
	PreviousLocation = GetActorLocation();
}

void ABaseCastProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ProjectileAge += DeltaSeconds;

	if (bHasImpacted)
	{
		return;
	}

	UpdateMagicLaunch(DeltaSeconds);
	UpdateMagicMotion(DeltaSeconds);
	UpdateVisualSpiral();

	if (!bHasLaunched)
	{
		PreviousLocation = GetActorLocation();
		return;
	}

	if (!bSweepBetweenFrames)
	{
		PreviousLocation = GetActorLocation();
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	if (PreviousLocation.Equals(CurrentLocation, KINDA_SMALL_NUMBER))
	{
		return;
	}

	if (TryCatchHeatSeekTarget(PreviousLocation, CurrentLocation))
	{
		return;
	}

	FHitResult Hit;
	FCollisionQueryParams QueryParams(FName(TEXT("MagicProjectileSweep")), false, this);
	for (AActor* Ignored : IgnoredActors)
	{
		if (Ignored)
		{
			QueryParams.AddIgnoredActor(Ignored);
		}
	}

	const FCollisionShape SweepShape = FCollisionShape::MakeSphere(Collision ? Collision->GetScaledSphereRadius() : 12.0f);
	bool bHit = false;

	if (bTraceOnlyEnemiesForSweeps)
	{
		bHit = GetWorld()->SweepSingleByObjectType(
			Hit,
			PreviousLocation,
			CurrentLocation,
			FQuat::Identity,
			FCollisionObjectQueryParams(ECC_Pawn),
			SweepShape,
			QueryParams);
	}
	else
	{
		bHit = GetWorld()->SweepSingleByChannel(
			Hit,
			PreviousLocation,
			CurrentLocation,
			FQuat::Identity,
			ECC_Visibility,
			SweepShape,
			QueryParams);
	}

	if (bHit && Hit.GetActor() && !IsIgnoredActor(Hit.GetActor()))
	{
		HandleActorHit(Hit.GetActor(), Hit);
		return;
	}

	PreviousLocation = CurrentLocation;
}

void ABaseCastProjectile::OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || IsIgnoredActor(OtherActor))
	{
		return;
	}

	HandleActorHit(OtherActor, SweepResult);
}

void ABaseCastProjectile::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || IsIgnoredActor(OtherActor))
	{
		return;
	}

	HandleActorHit(OtherActor, Hit);
}

void ABaseCastProjectile::OnProjectileImpacted_Implementation(AActor* HitActor, const FHitResult& Hit)
{
}

void ABaseCastProjectile::AddIgnoredActor(AActor* ActorToIgnore)
{
	if (!ActorToIgnore || ActorToIgnore == this || IgnoredActors.Contains(ActorToIgnore))
	{
		return;
	}

	IgnoredActors.Add(ActorToIgnore);

	if (Collision)
	{
		Collision->IgnoreActorWhenMoving(ActorToIgnore, true);
	}

	TArray<AActor*> AttachedActors;
	ActorToIgnore->GetAttachedActors(AttachedActors);
	for (AActor* AttachedActor : AttachedActors)
	{
		AddIgnoredActor(AttachedActor);
	}
}

bool ABaseCastProjectile::IsIgnoredActor(const AActor* Actor) const
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

bool ABaseCastProjectile::IsValidHeatSeekTarget(AActor* Candidate) const
{
	if (!Candidate || Candidate == this || Candidate == Caster || IsIgnoredActor(Candidate))
	{
		return false;
	}

	UEnemyHealthComponent* Health = Candidate->FindComponentByClass<UEnemyHealthComponent>();
	if (Health && Health->IsDeadOrOutOfHealth())
	{
		return false;
	}

	return Health || (!EnemyTargetTag.IsNone() && Candidate->ActorHasTag(EnemyTargetTag));
}

AActor* ABaseCastProjectile::GetLockedTargetFromCaster() const
{
	const AALSBaseCharacter* ALSCaster = Cast<AALSBaseCharacter>(Caster);
	if (!ALSCaster)
	{
		return nullptr;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(ALSCaster->GetController());
	const AALSPlayerCameraManager* CameraManager = PlayerController
		? Cast<AALSPlayerCameraManager>(PlayerController->PlayerCameraManager)
		: nullptr;

	if (!CameraManager || !CameraManager->bIsTargetLocked)
	{
		return nullptr;
	}

	return CameraManager->LockedTarget;
}

AActor* ABaseCastProjectile::FindBestTarget() const
{
	if (!Caster || !GetWorld())
	{
		DebugTargetingMessage(TEXT("Magic target scan skipped: no caster/world."), FColor::Red);
		return nullptr;
	}

	if (bPreferLockedTarget)
	{
		if (AActor* LockedTarget = GetLockedTargetFromCaster())
		{
			const bool bLockedTargetValid = IsValidHeatSeekTarget(LockedTarget);
			const FVector LockedAimLocation = bAlwaysHeatSeekEnemies ? GetHeatSeekAimLocation(LockedTarget) : GetTargetAimLocation(LockedTarget);
			const bool bLockedTargetInRange = FVector::Dist(Caster->GetActorLocation(), LockedAimLocation) <= TargetSearchRange;

			if (bLockedTargetValid && bLockedTargetInRange)
			{
				DebugTargetingMessage(FString::Printf(TEXT("Magic target acquired from lock-on: %s"), *LockedTarget->GetName()), FColor::Green);
				if (bDebugTargeting)
				{
					DrawDebugLine(GetWorld(), GetActorLocation(), LockedAimLocation, FColor::Green, false, TargetDebugDrawDuration, 0, 2.0f);
				}
				return LockedTarget;
			}
		}
	}

	if (!bAutoFindTarget && !bAlwaysHeatSeekEnemies)
	{
		DebugTargetingMessage(TEXT("Magic target scan skipped: bAutoFindTarget is false."), FColor::Yellow);
		return nullptr;
	}

	AActor* BestTarget = nullptr;
	float BestScore = FLT_MAX;
	int32 ScannedHealthComponents = 0;
	int32 ScannedOverlaps = 0;
	int32 ScannedTaggedActors = 0;
	int32 RejectedIgnored = 0;
	int32 RejectedDead = 0;
	int32 RejectedRange = 0;
	int32 RejectedForward = 0;

	const FVector SearchOrigin = bHasLaunched ? GetActorLocation() : Caster->GetActorLocation();
	const FVector Forward = GetCasterAimForward();
	const bool bIgnoreForwardCone = bAlwaysHeatSeekEnemies || TargetForwardDotThreshold <= -1.0f;

	auto ConsiderCandidate = [&](AActor* Candidate)
	{
		if (!Candidate || Candidate == this || Candidate == Caster || IsIgnoredActor(Candidate))
		{
			++RejectedIgnored;
			return;
		}

		UEnemyHealthComponent* Health = Candidate->FindComponentByClass<UEnemyHealthComponent>();
		const bool bHasEnemyTag = !EnemyTargetTag.IsNone() && Candidate->ActorHasTag(EnemyTargetTag);
		if (!Health && !bHasEnemyTag)
		{
			++RejectedIgnored;
			return;
		}

		if (Health && Health->IsDeadOrOutOfHealth())
		{
			++RejectedDead;
			return;
		}

		const FVector TargetLocation = bAlwaysHeatSeekEnemies ? GetHeatSeekAimLocation(Candidate) : GetTargetAimLocation(Candidate);
		const FVector ToTarget = TargetLocation - SearchOrigin;
		const float Distance = ToTarget.Size();
		if (Distance <= KINDA_SMALL_NUMBER || Distance > TargetSearchRange)
		{
			++RejectedRange;
			return;
		}

		const float ForwardDot = FVector::DotProduct(Forward, ToTarget.GetSafeNormal());
		if (!bIgnoreForwardCone && ForwardDot < TargetForwardDotThreshold)
		{
			++RejectedForward;
			return;
		}

		const float Score = bAlwaysHeatSeekEnemies ? Distance : Distance + ((1.0f - ForwardDot) * 600.0f);
		if (Score < BestScore)
		{
			BestScore = Score;
			BestTarget = Candidate;
		}
	};

	for (TObjectIterator<UEnemyHealthComponent> It; It; ++It)
	{
		UEnemyHealthComponent* Health = *It;
		if (!Health || Health->GetWorld() != GetWorld())
		{
			continue;
		}

		++ScannedHealthComponents;
		ConsiderCandidate(Health->GetOwner());
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams(FName(TEXT("MagicHeatSeekTargetScan")), false, this);
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(Caster);
	for (AActor* Ignored : IgnoredActors)
	{
		if (Ignored)
		{
			QueryParams.AddIgnoredActor(Ignored);
		}
	}

	const FCollisionShape SearchShape = FCollisionShape::MakeSphere(TargetSearchRange);
	if (GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		SearchOrigin,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		SearchShape,
		QueryParams))
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			++ScannedOverlaps;
			ConsiderCandidate(Overlap.GetActor());
		}
	}

	if (!EnemyTargetTag.IsNone())
	{
		TArray<AActor*> TaggedActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), EnemyTargetTag, TaggedActors);
		for (AActor* TaggedActor : TaggedActors)
		{
			++ScannedTaggedActors;
			ConsiderCandidate(TaggedActor);
		}
	}

	if (bDebugTargeting)
	{
		DrawDebugSphere(GetWorld(), SearchOrigin, TargetSearchRange, 32, BestTarget ? FColor::Green : FColor::Red, false, TargetDebugDrawDuration, 0, 1.5f);
		if (BestTarget)
		{
			const FVector BestTargetLocation = bAlwaysHeatSeekEnemies ? GetHeatSeekAimLocation(BestTarget) : GetTargetAimLocation(BestTarget);
			DrawDebugLine(GetWorld(), SearchOrigin, BestTargetLocation, FColor::Green, false, TargetDebugDrawDuration, 0, 2.5f);
			DrawDebugSphere(GetWorld(), BestTargetLocation, 28.0f, 12, FColor::Green, false, TargetDebugDrawDuration, 0, 2.0f);
		}

		const FString TargetName = BestTarget ? BestTarget->GetName() : TEXT("none");
		DebugTargetingMessage(
			FString::Printf(TEXT("Magic target scan: health=%d overlaps=%d tagged=%d ignored=%d dead=%d range=%d forward=%d target=%s"),
				ScannedHealthComponents,
				ScannedOverlaps,
				ScannedTaggedActors,
				RejectedIgnored,
				RejectedDead,
				RejectedRange,
				RejectedForward,
				*TargetName),
			BestTarget ? FColor::Green : FColor::Red);
	}

	return BestTarget;
}

void ABaseCastProjectile::DebugTargetingMessage(const FString& Message, const FColor& Color) const
{
	if (!bDebugTargeting)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()), TargetDebugDrawDuration, Color, Message);
	}
}

USceneComponent* ABaseCastProjectile::GetTargetHomingComponent(AActor* TargetActor) const
{
	if (!TargetActor)
	{
		return nullptr;
	}

	if (!TargetSocketName.IsNone())
	{
		if (USkeletalMeshComponent* Mesh = TargetActor->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (Mesh->DoesSocketExist(TargetSocketName))
			{
				return Mesh;
			}
		}
	}

	return TargetActor->GetRootComponent();
}

FVector ABaseCastProjectile::GetTargetAimLocation(AActor* TargetActor) const
{
	if (!TargetActor)
	{
		return FVector::ZeroVector;
	}

	if (!TargetSocketName.IsNone())
	{
		if (USkeletalMeshComponent* Mesh = TargetActor->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (Mesh->DoesSocketExist(TargetSocketName))
			{
				return Mesh->GetSocketLocation(TargetSocketName);
			}
		}
	}

	FVector AimLocation = TargetActor->GetActorLocation() + FVector(0.0f, 0.0f, TargetFallbackHeightOffset);
	if (TrackingPredictionTime > 0.0f)
	{
		AimLocation += TargetActor->GetVelocity() * TrackingPredictionTime;
	}

	return AimLocation;
}

FVector ABaseCastProjectile::GetHeatSeekAimLocation(AActor* TargetActor) const
{
	if (!TargetActor)
	{
		return FVector::ZeroVector;
	}

	if (!bHeatSeekAimAtCenterMass)
	{
		return GetTargetAimLocation(TargetActor);
	}

	FVector Origin = FVector::ZeroVector;
	FVector Extents = FVector::ZeroVector;
	TargetActor->GetActorBounds(false, Origin, Extents);

	FVector AimLocation = Extents.IsNearlyZero()
		? TargetActor->GetActorLocation() + FVector(0.0f, 0.0f, TargetFallbackHeightOffset * 0.5f)
		: Origin + FVector(0.0f, 0.0f, HeatSeekCenterMassZOffset);

	if (TrackingPredictionTime > 0.0f)
	{
		AimLocation += TargetActor->GetVelocity() * TrackingPredictionTime;
	}

	return AimLocation;
}

FVector ABaseCastProjectile::GetCasterAimForward() const
{
	if (const AALSBaseCharacter* ALSCaster = Cast<AALSBaseCharacter>(Caster))
	{
		if (const AController* Controller = ALSCaster->GetController())
		{
			FRotator AimRotation = Controller->GetControlRotation();
			AimRotation.Pitch = 0.0f;
			AimRotation.Roll = 0.0f;
			return AimRotation.Vector().GetSafeNormal();
		}

		return ALSCaster->GetActorForwardVector();
	}

	FRotator ActorRotation = GetActorRotation();
	ActorRotation.Pitch = 0.0f;
	ActorRotation.Roll = 0.0f;
	return ActorRotation.Vector().GetSafeNormal();
}

void ABaseCastProjectile::RefreshHomingTarget()
{
	const bool bShouldHeatSeek = bUseHoming || bAlwaysHeatSeekEnemies;
	if (!bShouldHeatSeek || !ProjectileMovement)
	{
		return;
	}

	if (!IsValidHeatSeekTarget(HomingTarget))
	{
		HomingTarget = FindBestTarget();
	}

	USceneComponent* HomingComponent = GetTargetHomingComponent(HomingTarget);
	ProjectileMovement->bIsHomingProjectile = false;
	ProjectileMovement->HomingTargetComponent = HomingComponent;
	ProjectileMovement->HomingAccelerationMagnitude = HomingAccelerationMagnitude;
}

void ABaseCastProjectile::ResetMagicLaunchState()
{
	if (Caster && MinimumSpawnHeightAboveCaster > 0.0f)
	{
		const float MinZ = Caster->GetActorLocation().Z + MinimumSpawnHeightAboveCaster;
		if (GetActorLocation().Z < MinZ)
		{
			FVector SafeLocation = GetActorLocation();
			SafeLocation.Z = MinZ;
			SetActorLocation(SafeLocation, false);
		}
	}

	SpawnLocation = GetActorLocation();
	SpawnAimRotation = GetActorRotation();
	ProjectileAge = 0.0f;
	TimeSinceLaunch = 0.0f;
	TimeSinceTargetAcquire = 0.0f;
	bHasLaunched = !bUseMagicLaunchStyle || LaunchDelay <= 0.0f;
	SpiralPhase = FMath::FRandRange(0.0f, UE_TWO_PI);

	const FVector Forward = GetActorForwardVector();
	SpiralRight = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
	if (SpiralRight.IsNearlyZero())
	{
		SpiralRight = GetActorRightVector();
	}
	SpiralUp = FVector::CrossProduct(Forward, SpiralRight).GetSafeNormal();
	if (SpiralUp.IsNearlyZero())
	{
		SpiralUp = GetActorUpVector();
	}

	if (ProjectileMovement)
	{
		if (bHasLaunched)
		{
			SetActorRotation(GetForwardLaunchDirection().Rotation());
			ProjectileMovement->Activate(true);
			ApplyInitialVelocity();
		}
		else
		{
			ProjectileMovement->StopMovementImmediately();
			ProjectileMovement->Deactivate();
		}
	}

	if (Collision)
	{
		Collision->SetCollisionEnabled(bHasLaunched ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	}
}

void ABaseCastProjectile::UpdateMagicLaunch(float DeltaSeconds)
{
	if (!bUseMagicLaunchStyle || bHasLaunched || bHasImpacted)
	{
		return;
	}

	const float DelayAlpha = LaunchDelay > 0.0f ? FMath::Clamp(ProjectileAge / LaunchDelay, 0.0f, 1.0f) : 1.0f;
	const float EaseAlpha = FMath::InterpEaseOut(0.0f, 1.0f, DelayAlpha, 2.4f);
	const FVector FloatLocation = SpawnLocation + GetActorTransform().TransformVectorNoScale(LaunchFloatOffset) * EaseAlpha;
	SetActorLocation(FloatLocation, false);

	if (DelayAlpha >= 1.0f)
	{
		LaunchProjectile();
	}
}

void ABaseCastProjectile::LaunchProjectile()
{
	if (bHasLaunched)
	{
		return;
	}

	bHasLaunched = true;
	TimeSinceLaunch = 0.0f;

	if (Collision)
	{
		Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	if (ProjectileMovement)
	{
		RefreshHomingTarget();
		SetActorRotation(GetForwardLaunchDirection().Rotation());
		ProjectileMovement->Activate(true);
		ApplyInitialVelocity();
	}

	PreviousLocation = GetActorLocation();
}

void ABaseCastProjectile::UpdateMagicMotion(float DeltaSeconds)
{
	if (!ProjectileMovement || !bHasLaunched || bHasImpacted)
	{
		return;
	}

	TimeSinceLaunch += DeltaSeconds;
	TimeSinceTargetAcquire += DeltaSeconds;

	const bool bShouldHeatSeek = bUseHoming || bAlwaysHeatSeekEnemies;
	if (bShouldHeatSeek && !IsValidHeatSeekTarget(HomingTarget))
	{
		HomingTarget = FindBestTarget();
	}

	if (bShouldHeatSeek && (bContinuouslyAcquireTargets || !HomingTarget) && TimeSinceTargetAcquire >= TargetAcquireInterval)
	{
		TimeSinceTargetAcquire = 0.0f;

		if (!IsValidHeatSeekTarget(HomingTarget))
		{
			HomingTarget = FindBestTarget();
		}
	}

	const float SpeedAlpha = SpeedRampDuration > 0.0f
		? FMath::Clamp(TimeSinceLaunch / SpeedRampDuration, 0.0f, 1.0f)
		: 1.0f;
	const float EasedSpeedAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, SpeedAlpha, 2.0f);
	const float CurrentSpeed = FMath::Lerp(InitialSpeed * InitialLaunchSpeedRatio, MaxSpeed, EasedSpeedAlpha);

	FVector CurrentDirection = ProjectileMovement->Velocity.GetSafeNormal();
	if (CurrentDirection.IsNearlyZero())
	{
		CurrentDirection = GetActorForwardVector();
	}

	FVector DesiredDirection = CurrentDirection;
	if (bShouldHeatSeek && HomingTarget && (bAlwaysHeatSeekEnemies || TimeSinceLaunch >= HomingDelayAfterLaunch))
	{
		DesiredDirection = GetDesiredLaunchDirection();
		if (bAlwaysHeatSeekEnemies)
		{
			const float DistanceToTarget = FVector::Dist(GetActorLocation(), GetHeatSeekAimLocation(HomingTarget));
			DesiredDirection = GetMagicalHeatSeekDirection(DesiredDirection, DistanceToTarget);

			if (HeatSeekTurnSharpness > 0.0f)
			{
				DesiredDirection = FMath::VInterpNormalRotationTo(CurrentDirection, DesiredDirection, DeltaSeconds, HeatSeekTurnSharpness);
			}
		}
		else
		{
			const float HomingAlpha = HomingRampDuration > 0.0f
				? FMath::Clamp(TimeSinceLaunch / HomingRampDuration, 0.0f, 1.0f)
				: 1.0f;
			const float CurrentTurnSpeed = HomingTurnSpeed * FMath::InterpEaseIn(0.0f, 1.0f, HomingAlpha, 2.0f);
			DesiredDirection = FMath::VInterpNormalRotationTo(CurrentDirection, DesiredDirection, DeltaSeconds, CurrentTurnSpeed);
		}
	}

	ProjectileMovement->Velocity = DesiredDirection * CurrentSpeed;
	if (!DesiredDirection.IsNearlyZero())
	{
		SetActorRotation(DesiredDirection.Rotation());
	}

	if (bShouldHeatSeek)
	{
		const float HomingAlpha = HomingRampDuration > 0.0f
			? FMath::Clamp(TimeSinceLaunch / HomingRampDuration, 0.0f, 1.0f)
			: 1.0f;
		ProjectileMovement->HomingAccelerationMagnitude = HomingAccelerationMagnitude * FMath::InterpEaseIn(0.0f, 1.0f, HomingAlpha, 2.0f);
	}
}

bool ABaseCastProjectile::TryCatchHeatSeekTarget(const FVector& TraceStart, const FVector& TraceEnd)
{
	if (!bAlwaysHeatSeekEnemies || !IsValidHeatSeekTarget(HomingTarget) || HeatSeekCatchRadius <= 0.0f)
	{
		return false;
	}

	const FVector TargetLocation = GetHeatSeekAimLocation(HomingTarget);
	const FVector Segment = TraceEnd - TraceStart;
	const float SegmentLengthSquared = Segment.SizeSquared();
	const float SegmentAlpha = SegmentLengthSquared > KINDA_SMALL_NUMBER
		? FMath::Clamp(FVector::DotProduct(TargetLocation - TraceStart, Segment) / SegmentLengthSquared, 0.0f, 1.0f)
		: 1.0f;
	const FVector ClosestPoint = TraceStart + Segment * SegmentAlpha;

	if (FVector::DistSquared(ClosestPoint, TargetLocation) > FMath::Square(HeatSeekCatchRadius))
	{
		return false;
	}

	FHitResult Hit;
	Hit.Location = ClosestPoint;
	Hit.ImpactPoint = TargetLocation;
	Hit.ImpactNormal = (TraceStart - TargetLocation).GetSafeNormal();
	Hit.Normal = Hit.ImpactNormal;

	HandleEnemyHit(HomingTarget, Hit);
	return true;
}

FVector ABaseCastProjectile::GetMagicalHeatSeekDirection(const FVector& DirectDirection, float DistanceToTarget) const
{
	if (!bUseMagicalHeatSeekPath || DirectDirection.IsNearlyZero())
	{
		return DirectDirection;
	}

	const float CloseFade = HeatSeekCloseFadeDistance > 0.0f
		? FMath::Clamp((DistanceToTarget - HeatSeekCatchRadius) / HeatSeekCloseFadeDistance, 0.0f, 1.0f)
		: 1.0f;
	const float LaunchFade = FMath::Clamp(TimeSinceLaunch / 0.28f, 0.0f, 1.0f);
	const float MagicAlpha = CloseFade * LaunchFade;
	if (MagicAlpha <= KINDA_SMALL_NUMBER)
	{
		return DirectDirection;
	}

	FVector SideAxis = FVector::CrossProduct(FVector::UpVector, DirectDirection).GetSafeNormal();
	if (SideAxis.IsNearlyZero())
	{
		SideAxis = GetActorRightVector();
	}

	FVector LiftAxis = FVector::CrossProduct(DirectDirection, SideAxis).GetSafeNormal();
	if (LiftAxis.IsNearlyZero())
	{
		LiftAxis = FVector::UpVector;
	}

	const float Phase = SpiralPhase + TimeSinceLaunch * HeatSeekWeaveFrequency;
	const FVector Weave =
		(SideAxis * FMath::Sin(Phase) * HeatSeekWeaveStrength) +
		(LiftAxis * FMath::Cos(Phase * 0.72f) * HeatSeekWeaveStrength * 0.55f) +
		(FVector::UpVector * HeatSeekVerticalLiftStrength);

	const FVector MagicalDirection = (DirectDirection + Weave * MagicAlpha).GetSafeNormal();
	return MagicalDirection.IsNearlyZero() ? DirectDirection : MagicalDirection;
}

void ABaseCastProjectile::UpdateVisualSpiral()
{
	if (!bUseVisualSpiral || !bHasLaunched || bHasImpacted)
	{
		if (ProjectileMesh)
		{
			ProjectileMesh->SetRelativeLocation(FVector::ZeroVector);
		}
		if (TrailFX)
		{
			TrailFX->SetRelativeLocation(FVector::ZeroVector);
		}
		return;
	}

	const float FadeAlpha = SpiralFadeInDuration > 0.0f
		? FMath::Clamp(TimeSinceLaunch / SpiralFadeInDuration, 0.0f, 1.0f)
		: 1.0f;
	const float Phase = SpiralPhase + TimeSinceLaunch * SpiralFrequency;
	const FVector Offset = (SpiralRight * FMath::Cos(Phase) + SpiralUp * FMath::Sin(Phase)) * SpiralAmplitude * FadeAlpha;

	if (ProjectileMesh)
	{
		ProjectileMesh->SetRelativeLocation(Offset);
	}
	if (TrailFX)
	{
		TrailFX->SetRelativeLocation(Offset * 0.5f);
	}
}

FVector ABaseCastProjectile::GetDesiredLaunchDirection() const
{
	if (HomingTarget)
	{
		const FVector TargetLocation = bAlwaysHeatSeekEnemies ? GetHeatSeekAimLocation(HomingTarget) : GetTargetAimLocation(HomingTarget);
		const FVector ToTarget = (TargetLocation - GetActorLocation()).GetSafeNormal();
		if (!ToTarget.IsNearlyZero())
		{
			return ToTarget;
		}
	}

	return GetForwardLaunchDirection();
}

FVector ABaseCastProjectile::GetForwardLaunchDirection() const
{
	FRotator LaunchRotation = SpawnAimRotation;
	LaunchRotation.Pitch = FMath::Max(LaunchRotation.Pitch, FMath::Max(LaunchPitchDegrees, MinimumLaunchPitch));
	LaunchRotation.Roll = 0.0f;
	return LaunchRotation.Vector().GetSafeNormal();
}

void ABaseCastProjectile::ApplyInitialVelocity()
{
	if (!ProjectileMovement)
	{
		return;
	}

	ProjectileMovement->InitialSpeed = InitialSpeed;
	ProjectileMovement->MaxSpeed = MaxSpeed;
	ProjectileMovement->bRotationFollowsVelocity = bRotationFollowsVelocity;
	ProjectileMovement->ProjectileGravityScale = ProjectileGravityScale;
	const float StartingSpeed = bUseMagicLaunchStyle ? InitialSpeed * InitialLaunchSpeedRatio : InitialSpeed;
	const FVector LaunchDirection = GetForwardLaunchDirection();
	ProjectileMovement->Velocity = LaunchDirection * StartingSpeed;
	SetActorRotation(LaunchDirection.Rotation());

	if (bUseHoming || bAlwaysHeatSeekEnemies)
	{
		ProjectileMovement->HomingAccelerationMagnitude = bUseMagicLaunchStyle ? 0.0f : HomingAccelerationMagnitude;
	}
}

void ABaseCastProjectile::HandleActorHit(AActor* HitActor, const FHitResult& Hit)
{
	if (!HitActor || bHasImpacted)
	{
		return;
	}

	if (HitActor->FindComponentByClass<UEnemyHealthComponent>())
	{
		HandleEnemyHit(HitActor, Hit);
		return;
	}

	HandleWorldHit(HitActor, Hit);
}

void ABaseCastProjectile::HandleEnemyHit(AActor* HitActor, const FHitResult& Hit)
{
	if (!HitActor)
	{
		return;
	}

	if (!bAllowMultipleEnemyHits && DamagedActors.Contains(HitActor))
	{
		return;
	}

	if (UEnemyHealthComponent* HealthComp = HitActor->FindComponentByClass<UEnemyHealthComponent>())
	{
		HealthComp->TakeDamage(DamageAmount);
		DamagedActors.Add(HitActor);
	}

	SpawnImpactFX(Hit);
	OnProjectileImpacted(HitActor, Hit);

	if (bDestroyOnEnemyHit)
	{
		FinishProjectile();
	}
}

void ABaseCastProjectile::HandleWorldHit(AActor* HitActor, const FHitResult& Hit)
{
	SpawnImpactFX(Hit);
	OnProjectileImpacted(HitActor, Hit);

	if (bDestroyOnWorldHit)
	{
		FinishProjectile();
	}
}

void ABaseCastProjectile::SpawnImpactFX(const FHitResult& Hit) const
{
	const FVector EffectLocation = Hit.ImpactPoint.IsNearlyZero() ? GetActorLocation() : FVector(Hit.ImpactPoint);
	const FRotator EffectRotation = Hit.ImpactNormal.IsNearlyZero()
		? (-GetActorForwardVector()).Rotation()
		: Hit.ImpactNormal.Rotation();

	if (ImpactNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactNiagara, EffectLocation, EffectRotation);
	}

	if (ImpactParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, EffectLocation, EffectRotation);
	}
}

void ABaseCastProjectile::StopProjectile()
{
	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}

	if (Collision)
	{
		Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (TrailFX)
	{
		TrailFX->Deactivate();
	}
}

void ABaseCastProjectile::FinishProjectile()
{
	if (bHasImpacted)
	{
		return;
	}

	bHasImpacted = true;
	StopProjectile();

	if (DestroyDelayAfterImpact <= 0.0f)
	{
		Destroy();
		return;
	}

	SetLifeSpan(DestroyDelayAfterImpact);
}
