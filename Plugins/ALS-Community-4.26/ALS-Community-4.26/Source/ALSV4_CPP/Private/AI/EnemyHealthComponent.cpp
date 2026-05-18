// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EnemyHealthComponent.h"
#include "AI/EnemyCombatComponent.h"
#include "AI/EnemyWolfCombatComponent.h"
#include "Components/WidgetComponent.h"
#include "AI/WBP_EnemyHealthBar.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/ALSBaseCharacter.h"
#include "Character/ALSPlayerCameraManager.h"
#include "Blueprint/UserWidget.h"
#include "BrainComponent.h"

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
	if (bIsDead)
	{
		return;
	}

	if (CurrentHealth <= 0.0f)
	{
		HandleDeath();
		return;
	}

	if (bIsInvincible)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	UpdateHealthBar();

	AActor* PlayerActor = UGameplayStatics::GetPlayerCharacter(this, 0);

	if (UEnemyCombatComponent* CombatComponent = GetOwner()
		? GetOwner()->FindComponentByClass<UEnemyCombatComponent>()
		: nullptr)
	{
		CombatComponent->HandleOwnerHit(PlayerActor);
	}
	else if (UEnemyWolfCombatComponent* WolfCombatComponent = GetOwner()
		? GetOwner()->FindComponentByClass<UEnemyWolfCombatComponent>()
		: nullptr)
	{
		WolfCombatComponent->HandleOwnerHit(PlayerActor);
	}

	// === Set WasHit to true on blackboard ===
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
		{
			if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
			{
				BB->SetValueAsBool("WasHit", true);
				if (PlayerActor)
				{
					BB->SetValueAsObject("TargetActor", PlayerActor);
				}
			}
		}
	}

	if (CurrentHealth <= 0.0f && !bIsDead)
	{
		HandleDeath();
	}
}

void UEnemyHealthComponent::HandleDeath()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	bIsInvincible = false;
	CurrentHealth = 0.0f;
	UpdateHealthBar();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(InvincibilityTimerHandle);
	}

	if (AActor* OwnerActor = GetOwner())
	{
		if (UEnemyCombatComponent* CombatComponent = OwnerActor->FindComponentByClass<UEnemyCombatComponent>())
		{
			CombatComponent->HandleOwnerDeath();
		}
		else if (UEnemyWolfCombatComponent* WolfCombatComponent = OwnerActor->FindComponentByClass<UEnemyWolfCombatComponent>())
		{
			WolfCombatComponent->HandleOwnerDeath();
		}

		if (APawn* OwnerPawn = Cast<APawn>(OwnerActor))
		{
			if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
			{
				AIController->ClearFocus(EAIFocusPriority::Gameplay);

				if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
				{
					BB->SetValueAsBool("WasHit", false);
					BB->SetValueAsBool("ShouldDodge", false);
					BB->SetValueAsBool("ShouldCircle", false);
					BB->SetValueAsBool("IsInAttackRange", false);
					BB->ClearValue("TargetActor");
				}

				if (UBrainComponent* Brain = AIController->GetBrainComponent())
				{
					Brain->StopLogic(TEXT("Enemy died"));
				}
			}
		}
	}

	// Drop runes visually
	if (RuneDropFX)
	{
		FVector EnemyLocation = GetOwner()->GetActorLocation();
		FVector PlayerLocation = FVector::ZeroVector;

		if (ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(this, 0))
		{
			PlayerLocation = PlayerChar->GetActorLocation();
		}

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
			FTimerHandle DelayHandle;
			FVector TargetLocation = PlayerLocation;

			GetWorld()->GetTimerManager().SetTimer(DelayHandle, [NiagaraComp, TargetLocation]()
				{
					if (NiagaraComp && !NiagaraComp->IsBeingDestroyed())
					{
						NiagaraComp->SetVariableVec3(FName("ActorLocation"), TargetLocation);
					}
				}, .7f, false);
		}
	}

	if (AALSBaseCharacter* Player = Cast<AALSBaseCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		if (UPlayerStatsComponent* Stats = Player->FindComponentByClass<UPlayerStatsComponent>())
		{
			Stats->AddRunes(RunesToDrop);
		}
	}

	ShowHealthBar(false);
	ClearLockOnIfTargetDies();

	OnDeath.Broadcast();
}

void UEnemyHealthComponent::SetInvincibleForDuration(float Duration)
{
	if (!GetWorld() || Duration <= 0.0f || bIsDead)
	{
		return;
	}

	bIsInvincible = true;
	GetWorld()->GetTimerManager().ClearTimer(InvincibilityTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		InvincibilityTimerHandle,
		this,
		&UEnemyHealthComponent::ResetInvincibility,
		Duration,
		false
	);
}

void UEnemyHealthComponent::ResetInvincibility()
{
	bIsInvincible = false;
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
