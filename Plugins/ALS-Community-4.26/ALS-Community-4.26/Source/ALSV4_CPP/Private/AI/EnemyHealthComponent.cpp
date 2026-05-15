// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EnemyHealthComponent.h"
#include "Components/WidgetComponent.h"
#include "AI/WBP_EnemyHealthBar.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/ALSBaseCharacter.h"
#include "Character/ALSPlayerCameraManager.h"
#include "Blueprint/UserWidget.h"

UEnemyHealthComponent::UEnemyHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;

	CacheHealthBarWidget();
	UpdateHealthBar(); // Initialize UI
}

void UEnemyHealthComponent::TakeDamage(float DamageAmount)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	UpdateHealthBar();

	// === Set WasHit to true on blackboard ===
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
		{
			if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
			{
				BB->SetValueAsBool("WasHit", true);
			}
		}
	}

	if (CurrentHealth <= 0.0f && !bIsDead)
	{
		bIsDead = true;

		// Drop runes visually
		if (RuneDropFX)
		{
			FVector EnemyLocation = GetOwner()->GetActorLocation();
			FVector PlayerLocation = FVector::ZeroVector;

			if (ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(this, 0))
			{
				PlayerLocation = PlayerChar->GetActorLocation();
			}

			// Spawn Niagara at enemy location
			UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				RuneDropFX,
				EnemyLocation,
				FRotator::ZeroRotator,
				FVector(1.f),
				true, true, ENCPoolMethod::AutoRelease
			);

			if (NiagaraComp)
			{
				// Delay setting the ActorLocation to prevent instant collection
				FTimerHandle DelayHandle;
				FVector TargetLocation = PlayerLocation;

				GetWorld()->GetTimerManager().SetTimer(DelayHandle, [NiagaraComp, TargetLocation]()
					{
						if (NiagaraComp && !NiagaraComp->IsBeingDestroyed())
						{
							NiagaraComp->SetVariableVec3(FName("ActorLocation"), TargetLocation);
						}
					}, .7f, false); // Delay in seconds before attraction starts (e.g., 1.0f)
			}
		}

		if (AALSBaseCharacter* Player = Cast<AALSBaseCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
		{
			if (UPlayerStatsComponent* Stats = Player->FindComponentByClass<UPlayerStatsComponent>())
			{
				Stats->AddRunes(RunesToDrop);
			}
		}

		// Optional: hide health bar
		ShowHealthBar(false);
		ClearLockOnIfTargetDies();

		OnDeath.Broadcast();
	}
}

void UEnemyHealthComponent::ClearLockOnIfTargetDies()
{
	if (AALSBaseCharacter* PlayerCharacter = Cast<AALSBaseCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		if (APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController()))
		{
			if (AALSPlayerCameraManager* CamManager = Cast<AALSPlayerCameraManager>(PC->PlayerCameraManager))
			{
				if (CamManager->LockedTarget == GetOwner())
				{
					CamManager->TargetLock(); // forcibly toggles OFF

					FTimerHandle RetargetHandle;
					GetWorld()->GetTimerManager().SetTimer(RetargetHandle, [CamManager]()
						{
							if (CamManager && !CamManager->bIsTargetLocked)
							{
								CamManager->TargetLock();
							}
						}, 0.05f, false);
				}
			}
		}
	}
}

void UEnemyHealthComponent::ShowHealthBar(bool bShow)
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(bShow);
	}
}

void UEnemyHealthComponent::CacheHealthBarWidget()
{
	if (!HealthBarWidget)
	{
		AActor* Owner = GetOwner();
		if (!Owner) return;

		HealthBarWidget = Owner->FindComponentByClass<UEnemyHealthBarWidgetComponent>();
	}

	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false); // start hidden
	}
}

void UEnemyHealthComponent::UpdateHealthBar()
{
	if (!HealthBarWidget) return;

	if (UUserWidget* Widget = HealthBarWidget->GetUserWidgetObject())
	{
		if (auto* HealthBar = Cast<UWBP_EnemyHealthBar>(Widget))
		{
			float Percent = FMath::Clamp(CurrentHealth / MaxHealth, 0.f, 1.f);
			HealthBar->SetHealthPercent(Percent);
		}
	}
}
