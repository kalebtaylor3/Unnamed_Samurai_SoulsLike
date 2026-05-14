// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponArrowProjectile.h"
#include "AI/EnemyHealthComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AWeaponArrowProjectile::AWeaponArrowProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	RootComponent = Collision;
	Collision->InitSphereRadius(8.0f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Collision->SetCollisionObjectType(ECC_WorldDynamic);
	Collision->SetCollisionResponseToAllChannels(ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	Collision->SetGenerateOverlapEvents(true);
	Collision->SetNotifyRigidBodyCollision(true);

	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
	ArrowMesh->SetupAttachment(Collision);
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 4500.0f;
	ProjectileMovement->MaxSpeed = 4500.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	InitialLifeSpan = 8.0f;
}

void AWeaponArrowProjectile::BeginPlay()
{
	Super::BeginPlay();

	Collision->OnComponentBeginOverlap.AddDynamic(this, &AWeaponArrowProjectile::OnArrowOverlap);
	Collision->OnComponentHit.AddDynamic(this, &AWeaponArrowProjectile::OnArrowHit);

	AddIgnoredActor(IgnoredActor);
	AddIgnoredActor(GetOwner());
	AddIgnoredActor(GetInstigator());

	PreviousLocation = GetActorLocation();
}

void AWeaponArrowProjectile::InitializeArrow(float InDamage, float InSpeed, AActor* InIgnoredActor)
{
	DamageAmount = InDamage;
	IgnoredActor = InIgnoredActor;
	IgnoredActors.Empty();
	DamagedActors.Empty();
	bStuck = false;
	PreviousLocation = GetActorLocation();

	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = InSpeed;
		ProjectileMovement->MaxSpeed = InSpeed;
		ProjectileMovement->Velocity = GetActorForwardVector() * InSpeed;
	}

	AddIgnoredActor(IgnoredActor);
	AddIgnoredActor(GetOwner());
	AddIgnoredActor(GetInstigator());
}

void AWeaponArrowProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bStuck)
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	if (!PreviousLocation.Equals(CurrentLocation, KINDA_SMALL_NUMBER))
	{
		FHitResult Hit;
		FCollisionQueryParams QueryParams(FName(TEXT("ArrowPawnSweep")), false, this);
		for (AActor* Ignored : IgnoredActors)
		{
			if (Ignored)
			{
				QueryParams.AddIgnoredActor(Ignored);
			}
		}

		const bool bHit = GetWorld()->SweepSingleByObjectType(
			Hit,
			PreviousLocation,
			CurrentLocation,
			FQuat::Identity,
			FCollisionObjectQueryParams(ECC_Pawn),
			FCollisionShape::MakeSphere(Collision ? Collision->GetScaledSphereRadius() : 8.0f),
			QueryParams);

		if (bHit && Hit.GetActor() && !IsIgnoredActor(Hit.GetActor()))
		{
			if (Hit.GetActor()->FindComponentByClass<UEnemyHealthComponent>())
			{
				HandleEnemyHit(Hit.GetActor(), Hit);
				return;
			}
		}
	}

	PreviousLocation = CurrentLocation;
}

void AWeaponArrowProjectile::OnArrowOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || IsIgnoredActor(OtherActor))
	{
		return;
	}

	if (OtherActor->FindComponentByClass<UEnemyHealthComponent>())
	{
		HandleEnemyHit(OtherActor, SweepResult);
	}
}

void AWeaponArrowProjectile::OnArrowHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || IsIgnoredActor(OtherActor))
	{
		return;
	}

	if (UEnemyHealthComponent* HealthComp = OtherActor->FindComponentByClass<UEnemyHealthComponent>())
	{
		HandleEnemyHit(OtherActor, Hit);
		return;
	}

	if (CanStickToHit(Hit))
	{
		StickArrowToHit(Hit);
	}
}

void AWeaponArrowProjectile::AddIgnoredActor(AActor* ActorToIgnore)
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

bool AWeaponArrowProjectile::IsIgnoredActor(const AActor* Actor) const
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

bool AWeaponArrowProjectile::CanStickToHit(const FHitResult& Hit) const
{
	const UPrimitiveComponent* HitComponent = Hit.GetComponent();
	if (!HitComponent)
	{
		return false;
	}

	return EnemyBodyTag.IsNone() || HitComponent->ComponentHasTag(EnemyBodyTag);
}

bool AWeaponArrowProjectile::TryFindEnemyBodyStickHit(AActor* HitActor, const FHitResult& OriginalHit,
	FHitResult& OutStickHit) const
{
	if (!HitActor || EnemyBodyTag.IsNone())
	{
		return false;
	}

	const FVector Forward = GetActorForwardVector();
	FVector TraceStart = OriginalHit.TraceStart;
	FVector TraceEnd = OriginalHit.TraceEnd;

	if (TraceStart.Equals(TraceEnd, KINDA_SMALL_NUMBER))
	{
		const FVector CurrentLocation = GetActorLocation();
		TraceStart = !PreviousLocation.IsNearlyZero() ? PreviousLocation : CurrentLocation - Forward * 100.0f;
		TraceEnd = CurrentLocation + Forward * 200.0f;
	}
	else
	{
		TraceStart -= Forward * 25.0f;
		TraceEnd += Forward * 75.0f;
	}

	FCollisionQueryParams QueryParams(FName(TEXT("ArrowEnemyBodyTrace")), false, this);
	for (AActor* Ignored : IgnoredActors)
	{
		if (Ignored)
		{
			QueryParams.AddIgnoredActor(Ignored);
		}
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	HitActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent || !PrimitiveComponent->ComponentHasTag(EnemyBodyTag))
		{
			continue;
		}

		FHitResult ComponentHit;
		if (PrimitiveComponent->LineTraceComponent(ComponentHit, TraceStart, TraceEnd, QueryParams))
		{
			OutStickHit = ComponentHit;
			return true;
		}
	}

	return false;
}

void AWeaponArrowProjectile::HandleEnemyHit(AActor* HitActor, const FHitResult& Hit)
{
	if (!HitActor)
	{
		return;
	}

	if (UEnemyHealthComponent* HealthComp = HitActor->FindComponentByClass<UEnemyHealthComponent>())
	{
		if (!DamagedActors.Contains(HitActor))
		{
			HealthComp->TakeDamage(DamageAmount);
			DamagedActors.Add(HitActor);
		}
	}

	FHitResult StickHit;
	if (CanStickToHit(Hit))
	{
		StickHit = Hit;
	}
	else if (!TryFindEnemyBodyStickHit(HitActor, Hit, StickHit))
	{
		return;
	}

	StickArrowToHit(StickHit);
}

void AWeaponArrowProjectile::StickArrowToHit(const FHitResult& Hit)
{
	if (bStuck)
	{
		return;
	}

	bStuck = true;

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}

	if (Collision)
	{
		Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (Hit.ImpactPoint.IsNearlyZero())
	{
		SetActorLocation(GetActorLocation());
	}
	else
	{
		SetActorLocation(Hit.ImpactPoint);
	}

	if (USceneComponent* HitComponent = Hit.GetComponent())
	{
		AttachToComponent(HitComponent, FAttachmentTransformRules::KeepWorldTransform, Hit.BoneName);
	}
}
