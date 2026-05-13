// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HeldWeaponBase.h"
#include "Components/BoxComponent.h"
#include "AI/EnemyHealthComponent.h"

AHeldWeaponBase::AHeldWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(Root);

	DamageHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageHitbox"));
	DamageHitbox->SetupAttachment(WeaponMesh);
	DamageHitbox->SetCollisionObjectType(ECC_WorldDynamic);
	DamageHitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageHitbox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // Detect enemy pawns
	DamageHitbox->SetGenerateOverlapEvents(true);
	DamageHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Off by default
}

void AHeldWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	DamageHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DamageHitbox->OnComponentBeginOverlap.AddDynamic(this, &AHeldWeaponBase::OnDamageHitboxOverlap);
}

void AHeldWeaponBase::EnableDamageCollision(float InDamageAmount)
{
	CurrentDamageAmount = InDamageAmount;
	AlreadyDamagedActors.Empty();

	DamageHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AHeldWeaponBase::DisableDamageCollision()
{
	DamageHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CurrentDamageAmount = 0.f;
	AlreadyDamagedActors.Empty();
}

void AHeldWeaponBase::OnDamageHitboxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetOwner() || AlreadyDamagedActors.Contains(OtherActor)) return;

	if (UEnemyHealthComponent* HealthComp = OtherActor->FindComponentByClass<UEnemyHealthComponent>())
	{
		HealthComp->TakeDamage(CurrentDamageAmount);
		const bool bKilledEnemy = HealthComp->IsDeadOrOutOfHealth();
		AlreadyDamagedActors.Add(OtherActor);

		// === Spawn hit particle ===
		if (HitEffect)
		{
			const FVector EffectScale = FVector(1.3f);
			const FVector WeaponHitboxLocation = DamageHitbox->GetComponentLocation();

			FVector HitLocation = SweepResult.ImpactPoint;
			if (!bFromSweep || HitLocation.IsNearlyZero())
			{
				HitLocation = OtherActor->GetActorLocation();

				if (OtherComp)
				{
					FVector ClosestPoint = FVector::ZeroVector;
					if (OtherComp->GetClosestPointOnCollision(WeaponHitboxLocation, ClosestPoint) > 0.f)
					{
						HitLocation = ClosestPoint;
					}
				}
			}

			const FVector HitDirection = (HitLocation - WeaponHitboxLocation).GetSafeNormal();
			FRotator EffectRotation = HitDirection.Rotation();
			EffectRotation.Pitch += 180.f; // Flip to face away from enemy

			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEffect, HitLocation, EffectRotation, EffectScale);
		}

		// === Hitstop ===
		const float HitstopDuration = bKilledEnemy ? 0.95f : 0.10f;
		const float TimeDilationAmount = 0.02f;

		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), TimeDilationAmount);

		FTimerHandle HitstopTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(HitstopTimerHandle, []()
			{
				UGameplayStatics::SetGlobalTimeDilation(GWorld, 1.0f); // Reset time scale
			}, HitstopDuration * TimeDilationAmount, false); // Use scaled time

		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, TEXT("Hit enemy!"));
	}
}

