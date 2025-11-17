// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EnemyHeldWeaponBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/PlayerStatsComponent.h"

AEnemyHeldWeaponBase::AEnemyHeldWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(Root);

	DamageHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageHitbox"));
	DamageHitbox->SetupAttachment(WeaponMesh);
	DamageHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DamageHitbox->SetCollisionObjectType(ECC_WorldDynamic);
	DamageHitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageHitbox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DamageHitbox->SetGenerateOverlapEvents(true);
}

void AEnemyHeldWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void AEnemyHeldWeaponBase::EnableDamageCollision(float InDamageAmount)
{
	CurrentDamageAmount = InDamageAmount;
	AlreadyDamagedActors.Empty();
	DamageHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemyHeldWeaponBase::DisableDamageCollision()
{
	DamageHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AlreadyDamagedActors.Empty();
}

void AEnemyHeldWeaponBase::OnDamageHitboxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetOwner() || AlreadyDamagedActors.Contains(OtherActor)) return;

	if (UPlayerStatsComponent* HealthComp = OtherActor->FindComponentByClass<UPlayerStatsComponent>())
	{
		if (!HealthComp->bIsInvincible)
		{
			HealthComp->TakeDamage(CurrentDamageAmount);
			AlreadyDamagedActors.Add(OtherActor);
			GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, TEXT("Hit Player!"));

			if (HitEffect)
			{
				FVector EffectScale = FVector(1.5f); // You can change this to whatever scale you want
				FVector BoxCenter = DamageHitbox->GetComponentLocation();
				FVector ToEnemy = OtherActor->GetActorLocation() - BoxCenter;
				FVector ToEnemyDir = ToEnemy.GetSafeNormal();

				BoxCenter += ToEnemyDir * 50.f; // Move 20 units closer *to the enemy*

				FRotator EffectRotation = ToEnemy.Rotation();  // Face toward the enemy
				EffectRotation.Pitch += 180.f; // Flip to face away from enemy

				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEffect, BoxCenter, EffectRotation, EffectScale);
			}

			// === Hitstop ===
			const float HitstopDuration = 0.05f;
			const float TimeDilationAmount = 0.01f;

			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), TimeDilationAmount);

			FTimerHandle HitstopTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(HitstopTimerHandle, []()
				{
					UGameplayStatics::SetGlobalTimeDilation(GWorld, 1.0f); // Reset time scale
				}, HitstopDuration * TimeDilationAmount, false); // Use scaled time
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage((int32)((uintptr_t)this), 0.f, FColor::Green,
			FString::Printf(TEXT("didnt work")));
	}
}
