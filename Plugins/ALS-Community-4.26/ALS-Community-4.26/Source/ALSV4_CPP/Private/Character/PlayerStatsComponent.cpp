// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PlayerStatsComponent.h"
#include "BonfireSaveGame.h"
#include "AI/EnemyCombatComponent.h"
#include "MyGameInstance.h"
#include "Character/ALSBaseCharacter.h"


UPlayerStatsComponent::UPlayerStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerStatsComponent::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AALSBaseCharacter* ALSChar = Cast<AALSBaseCharacter>(PC->GetCharacter()))
		{
			if (ALSChar->PlayerHUDWidget)
			{
				ALSChar->PlayerHUDWidget->UpdateRunes(CurrentRunes);
			}
		}
	}
}

void UPlayerStatsComponent::UseFP(float Amount)
{
	CurrentFP = FMath::Clamp(CurrentFP - Amount, 0.f, MaxFP);
	OnStatsChanged.Broadcast(CurrentHealth, MaxHealth, CurrentFP, MaxFP, CurrentStamina, MaxStamina, CurrentLevel, MaxLevel);
}

void UPlayerStatsComponent::UseStamina(float Amount)
{
	if (CurrentStamina < Amount)
	{
		NotifyStaminaExhausted();
	}

	CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.f, MaxStamina);
	OnStatsChanged.Broadcast(CurrentHealth, MaxHealth, CurrentFP, MaxFP, CurrentStamina, MaxStamina, CurrentLevel, MaxLevel);

	// Cancel any existing regen or delay
	GetWorld()->GetTimerManager().ClearTimer(StaminaRegenHandle);
	GetWorld()->GetTimerManager().ClearTimer(StaminaRegenDelayHandle);

	// Restart regen after delay
	GetWorld()->GetTimerManager().SetTimer(StaminaRegenDelayHandle, [this]()
		{
			RegenerateStamina(MaxStamina - CurrentStamina);
		}, 1.5f, false);
}

void UPlayerStatsComponent::NotifyStaminaExhausted()
{
	if (AALSBaseCharacter* OwnerChar = Cast<AALSBaseCharacter>(GetOwner()))
	{
		if (OwnerChar->PlayerHUDWidget)
		{
			OwnerChar->PlayerHUDWidget->PlayStaminaDeniedFeedback();
		}
	}
}

void UPlayerStatsComponent::RegenerateStamina(float Amount)
{
	if (Amount <= 0.f || CurrentStamina >= MaxStamina)
		return;

	// Cancel previous regen
	GetWorld()->GetTimerManager().ClearTimer(StaminaRegenHandle);

	// Spread the regen over 20 ticks
	RemainingStaminaToRegen = Amount;
	StaminaRegenPerTick = Amount / 30.f;

	GetWorld()->GetTimerManager().SetTimer(
		StaminaRegenHandle,
		this,
		&UPlayerStatsComponent::ApplyStaminaRegenTick,
		StaminaRegenTickRate,
		true
	);
}

void UPlayerStatsComponent::ApplyStaminaRegenTick()
{
	if (!bCanRegenStamina || RemainingStaminaToRegen <= 0.f || CurrentStamina >= MaxStamina)
	{
		GetWorld()->GetTimerManager().ClearTimer(StaminaRegenHandle);
		return;
	}

	const float ActualRegen = FMath::Min(StaminaRegenPerTick, RemainingStaminaToRegen);
	CurrentStamina = FMath::Clamp(CurrentStamina + ActualRegen, 0.f, MaxStamina);
	RemainingStaminaToRegen -= ActualRegen;

	OnStatsChanged.Broadcast(CurrentHealth, MaxHealth, CurrentFP, MaxFP, CurrentStamina, MaxStamina, CurrentLevel, MaxLevel);
}

void UPlayerStatsComponent::TakeDamage(float Amount)
{
	if (bIsInvincible)
		return; // No damage during i-frames

	if (CombatComponent && CombatComponent->bIsAttacking)
	{
		CombatComponent->InterruptAttack();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, TEXT("no combat component"));
	}

	// === Set being hit flag ===
	bBeingHit = true;

	// Clear existing timer if any
	GetWorld()->GetTimerManager().ClearTimer(BeingHitResetTimer);

	// Reset flag after delay
	GetWorld()->GetTimerManager().SetTimer(BeingHitResetTimer, this, &UPlayerStatsComponent::ResetBeingHitFlag, HitReactMontage->GetPlayLength(), false);

	CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.f, MaxHealth);
	OnStatsChanged.Broadcast(CurrentHealth, MaxHealth, CurrentFP, MaxFP, CurrentStamina, MaxStamina, CurrentLevel, MaxLevel);

	// === Play Hit React Animation ===
	if (HitReactMontage)
	{
		if (AALSBaseCharacter* OwnerChar = Cast<AALSBaseCharacter>(GetOwner()))
		{
			if (UAnimInstance* AnimInstance = OwnerChar->GetMesh()->GetAnimInstance())
			{
				AnimInstance->Montage_Play(HitReactMontage);

				// === Set being hit flag and start timer based on montage length ===
				bBeingHit = true;
				const float MontageLength = HitReactMontage->GetPlayLength();

				GetWorld()->GetTimerManager().SetTimer(
					BeingHitResetTimer,
					this,
					&UPlayerStatsComponent::ResetBeingHitFlag,
					MontageLength,
					false
				);
			}
		}
	}

	// === Death Check ===
	if (CurrentHealth <= 0.f)
	{
		HandlePlayerDeath();
	}
}

void UPlayerStatsComponent::HandlePlayerDeath()
{
	if (AALSBaseCharacter* Player = Cast<AALSBaseCharacter>(GetOwner()))
	{
		// === Clear any previous rune drop from GameInstance and SaveGame ===
		if (UMyGameInstance* GI = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			if (GI->bHasRuneDrop)
			{
				GI->bHasRuneDrop = false;
				GI->DroppedRuneAmount = 0;
				GI->DroppedRuneLocation = FVector::ZeroVector;

				if (UGameplayStatics::DoesSaveGameExist(TEXT("BonfireSlot"), 0))
				{
					if (UBonfireSaveGame* ExistingSave = Cast<UBonfireSaveGame>(
						UGameplayStatics::LoadGameFromSlot(TEXT("BonfireSlot"), 0)))
					{
						ExistingSave->bHasRuneDrop = false;
						ExistingSave->DroppedRuneAmount = 0;
						ExistingSave->DroppedRuneLocation = FVector::ZeroVector;
						UGameplayStatics::SaveGameToSlot(ExistingSave, TEXT("BonfireSlot"), 0);
					}
				}
			}
		}

		// === Handle dropping current runes ===
		if (CurrentRunes > 0)
		{
			FVector DropLocation = Player->GetActorLocation();
			int32 RunesToDrop = CurrentRunes;

			// Clear player runes
			CurrentRunes = 0;

			// Save new rune drop to GameInstance
			if (UMyGameInstance* GI = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(this)))
			{
				GI->bWasKilled = true;
				GI->bHasRuneDrop = true;
				GI->DroppedRuneAmount = RunesToDrop;
				GI->DroppedRuneLocation = DropLocation;
			}

			// Save new rune drop to disk
			UBonfireSaveGame* SaveData = nullptr;

			if (UGameplayStatics::DoesSaveGameExist(TEXT("BonfireSlot"), 0))
			{
				SaveData = Cast<UBonfireSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("BonfireSlot"), 0));
			}

			if (!SaveData)
			{
				SaveData = Cast<UBonfireSaveGame>(UGameplayStatics::CreateSaveGameObject(UBonfireSaveGame::StaticClass()));
			}

			if (SaveData)
			{
				SaveData->bHasRuneDrop = true;
				SaveData->DroppedRuneAmount = RunesToDrop;
				SaveData->DroppedRuneLocation = DropLocation;
				SaveData->CurrentRunes = 0; // Runes are lost
				UGameplayStatics::SaveGameToSlot(SaveData, TEXT("BonfireSlot"), 0);
			}
		}
	}

	OnPlayerDied.Broadcast();

	if (DeathScreenClass && !ActiveDeathScreen)
	{
		ActiveDeathScreen = CreateWidget<UUserWidget>(GetWorld(), DeathScreenClass);
		if (ActiveDeathScreen)
		{
			ActiveDeathScreen->AddToViewport();
		}
	}
}


void UPlayerStatsComponent::LoadGameFromBonfire()
{
	// ?? No save found? Just reload current level from scratch
	if (!UGameplayStatics::DoesSaveGameExist(TEXT("BonfireSlot"), 0))
	{
		UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
		return;
	}

	UBonfireSaveGame* SaveData = Cast<UBonfireSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("BonfireSlot"), 0));
	if (!SaveData)
	{
		UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName())); // safety fallback
		return;
	}

	FString CurrentMapName = UGameplayStatics::GetCurrentLevelName(this, true);

	// ??? If bonfire data is for a different level than we're in and player has *never* used a bonfire here, just reload
	if (SaveData->LevelName != CurrentMapName)
	{
		// If this is a fresh death in a level where no bonfire was touched
		if (!UGameplayStatics::GetStreamingLevel(GetWorld(), FName(*SaveData->LevelName)))
		{
			// ?? Just reload current map from scratch
			UGameplayStatics::OpenLevel(this, FName(*CurrentMapName));
			return;
		}
	}

	// ? Store data to GameInstance for soft reset
	if (UMyGameInstance* MyGI = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		MyGI->bShouldRestoreFromBonfire = true;
		MyGI->SavedBonfireLocation = SaveData->PlayerLocation;
		MyGI->SavedBonfireRotation = SaveData->PlayerRotation;
		MyGI->SavedHealth = SaveData->Health;
		MyGI->SavedMaxHealth = SaveData->MaxHealth;
		MyGI->SavedRunes = SaveData->CurrentRunes;
		MyGI->SavedLevel = SaveData->PlayerLevel;
		MyGI->SavedVigor = SaveData->VigorLevel;
		MyGI->SavedMind = SaveData->MindLevel;
		MyGI->SavedEndurance = SaveData->EnduranceLevel;
		MyGI->SavedLevelName = SaveData->LevelName;

		// ? Inventory
		if (const AALSBaseCharacter* OwnerChar = Cast<AALSBaseCharacter>(GetOwner()))
		{
			if (UInventoryComponent* Inventory = OwnerChar->FindComponentByClass<UInventoryComponent>())
			{
				MyGI->SavedBackpackWeapons = Inventory->BackpackWeapons;
				MyGI->SavedEquippedWeapons = Inventory->EquippedWeapons;
				MyGI->SavedEquippedIndex = Inventory->EquippedIndex;
			}
		}
	}

	// ?? Reload level (same or saved)
	const FString TargetLevel = (SaveData->LevelName != CurrentMapName)
		? SaveData->LevelName
		: CurrentMapName;

	UGameplayStatics::OpenLevel(this, FName(*TargetLevel));
}

void UPlayerStatsComponent::ResetInvincibility()
{
	bIsInvincible = false;
}

void UPlayerStatsComponent::ResetBeingHitFlag()
{
	bBeingHit = false;
}

void UPlayerStatsComponent::RestoreHealth(float Amount)
{
	if (Amount <= 0.f || CurrentHealth >= MaxHealth)
		return;

	// Stop previous healing if any
	GetWorld()->GetTimerManager().ClearTimer(HealthRegenHandle);

	// Define how much to heal per tick
	RemainingHealAmount = Amount;
	HealTickAmount = Amount / 30.f; // Heal in 20 small ticks

	GetWorld()->GetTimerManager().SetTimer(HealthRegenHandle, this, &UPlayerStatsComponent::ApplyHealthTick, HealTickRate, true);
}

void UPlayerStatsComponent::ApplyHealthTick()
{
	if (RemainingHealAmount <= 0.f || CurrentHealth >= MaxHealth)
	{
		GetWorld()->GetTimerManager().ClearTimer(HealthRegenHandle);
		return;
	}

	float ActualHeal = FMath::Min(HealTickAmount, RemainingHealAmount);
	CurrentHealth = FMath::Clamp(CurrentHealth + ActualHeal, 0.f, MaxHealth);
	RemainingHealAmount -= ActualHeal;

	OnStatsChanged.Broadcast(CurrentHealth, MaxHealth, CurrentFP, MaxFP, CurrentStamina, MaxStamina, CurrentLevel, MaxLevel);
}

void UPlayerStatsComponent::RestoreFP(float Amount)
{
	if (Amount <= 0.f || CurrentFP >= MaxFP)
		return;

	// Stop previous FP regen if any
	GetWorld()->GetTimerManager().ClearTimer(FPRegenHandle);

	// Set up regen values
	RemainingFPAmount = Amount;
	FPTickAmount = Amount / 30.f; // Spread across 20 ticks

	GetWorld()->GetTimerManager().SetTimer(FPRegenHandle, this, &UPlayerStatsComponent::ApplyFPTick, FPTickRate, true);
}

void UPlayerStatsComponent::ApplyFPTick()
{
	if (RemainingFPAmount <= 0.f || CurrentFP >= MaxFP)
	{
		GetWorld()->GetTimerManager().ClearTimer(FPRegenHandle);
		return;
	}

	float ActualRegen = FMath::Min(FPTickAmount, RemainingFPAmount);
	CurrentFP = FMath::Clamp(CurrentFP + ActualRegen, 0.f, MaxFP);
	RemainingFPAmount -= ActualRegen;

	OnStatsChanged.Broadcast(CurrentHealth, MaxHealth, CurrentFP, MaxFP, CurrentStamina, MaxStamina, CurrentLevel, MaxLevel);
}

void UPlayerStatsComponent::SetMaxValues(float NewHealth, float NewFP, float NewStamina)
{
	MaxHealth = NewHealth;
	MaxFP = NewFP;
	MaxStamina = NewStamina;

	CurrentHealth = FMath::Min(CurrentHealth, MaxHealth);
	CurrentFP = FMath::Min(CurrentFP, MaxFP);
	CurrentStamina = FMath::Min(CurrentStamina, MaxStamina);

	OnStatsChanged.Broadcast(CurrentHealth, MaxHealth, CurrentFP, MaxFP, CurrentStamina, MaxStamina, CurrentLevel, MaxLevel);
}

void UPlayerStatsComponent::IncreaseLevel(int32 HealthLevels, int32 FPLevels, int32 StaminaLevels)
{

	const int32 TotalPointsSpent = HealthLevels + FPLevels + StaminaLevels;
	if (TotalPointsSpent <= 0 || CurrentLevel >= MaxLevel)
		return;

	// Calculate rune cost for *next level(s)*
	int32 RuneCost = 0;
	for (int32 i = 1; i <= TotalPointsSpent; ++i)
	{
		RuneCost += GetRunesRequiredForLevel(CurrentLevel + i - 1);
	}

	if (!TrySpendRunes(RuneCost))
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Not enough runes!"));
		return;
	}

	// Apply stat boosts
	if (HealthLevels > 0)
	{

		float healthIncreasAmount;

		if (CurrentLevel < 40)
			healthIncreasAmount = 48.f;
		else if (CurrentLevel < 50)
			healthIncreasAmount = 26;
		else if (CurrentLevel < 60)
			healthIncreasAmount = 13;
		else
			healthIncreasAmount = 6;

		MaxHealth += HealthLevels * healthIncreasAmount;
		CurrentHealth = MaxHealth;
	}

	if (FPLevels > 0)
	{

		float fpIncreasAmount;

		if (CurrentLevel < 50)
			fpIncreasAmount = 6.f;
		else
			fpIncreasAmount = 2.f;


		MaxFP += FPLevels * fpIncreasAmount;
		CurrentFP = MaxFP;
	}

	if (StaminaLevels > 0)
	{
		float staminaIncreasAmount;

		if (CurrentLevel < 30)
			staminaIncreasAmount = 2.f;
		else
			staminaIncreasAmount = 1.f;

		MaxStamina += StaminaLevels * staminaIncreasAmount;
		CurrentStamina = MaxStamina;
	}

	// Only level up once (regardless of how many attributes changed)
	if (TotalPointsSpent > 0 && CurrentLevel < MaxLevel)
	{
		const int32 NewLevel = FMath::Clamp(CurrentLevel + TotalPointsSpent, 1, MaxLevel);

		VigorLevel += HealthLevels;
		MindLevel += FPLevels;
		EnduranceLevel += StaminaLevels;

		CurrentLevel = NewLevel;

		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green,
			FString::Printf(TEXT("Level Up! Now level %d"), CurrentLevel));
	}

	// Update HUD
	OnStatsChanged.Broadcast(CurrentHealth, MaxHealth, CurrentFP, MaxFP, CurrentStamina, MaxStamina, CurrentLevel, MaxLevel);
}

int32 UPlayerStatsComponent::GetRunesRequiredForLevel(int32 Level) const
{
	const float x = FMath::Max(0.f, ((Level + 81.f) - 92.f) * 0.02f);
	const float Runes = (x + 0.1f) * FMath::Pow(Level + 81.f, 2.f) + 1.f;
	return FMath::FloorToInt(Runes);
}

bool UPlayerStatsComponent::TrySpendRunes(int32 Amount)
{
	if (CurrentRunes >= Amount)
	{
		CurrentRunes -= Amount;
		OnRunesChanged.Broadcast(CurrentRunes, Amount);
		// Update HUD
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			if (AALSBaseCharacter* ALSChar = Cast<AALSBaseCharacter>(PC->GetCharacter()))
			{
				if (ALSChar->PlayerHUDWidget)
				{
					ALSChar->PlayerHUDWidget->UpdateRunes(CurrentRunes);
				}
			}
		}
		return true;
	}
	OnRunesChanged.Broadcast(CurrentRunes, Amount); // still update UI if failed
	return false;
}

void UPlayerStatsComponent::AddRunes(int32 Amount)
{
	CurrentRunes += Amount;

	// Optional: calculate cost of next level only
	int32 Needed = 0;
	for (int32 i = 1; i <= 1; ++i) // next level only
	{
		Needed += GetRunesRequiredForLevel(CurrentLevel + i - 1);
	}
	OnRunesChanged.Broadcast(CurrentRunes, Needed);

	// Update HUD
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AALSBaseCharacter* ALSChar = Cast<AALSBaseCharacter>(PC->GetCharacter()))
		{
			if (ALSChar->PlayerHUDWidget)
			{
				ALSChar->PlayerHUDWidget->UpdateRunes(CurrentRunes);
			}
		}
	}
}

void UPlayerStatsComponent::RecalculateDerivedStats()
{
	// === Base Stats (used at level 1) ===
	const float BaseHealth = 448.f;
	const float BaseFP = 62.f;
	const float BaseStamina = 50.f;

	// === Health ===
	MaxHealth = BaseHealth;
	for (int32 i = 2; i <= VigorLevel; ++i) // Start from level 2
	{
		if (i < 40)
			MaxHealth += 48.f;
		else if (i < 50)
			MaxHealth += 26.f;
		else if (i < 60)
			MaxHealth += 13.f;
		else
			MaxHealth += 6.f;
	}
	CurrentHealth = FMath::Clamp(CurrentHealth, 0.f, MaxHealth);

	// === FP ===
	MaxFP = BaseFP;
	for (int32 i = 2; i <= MindLevel; ++i)
	{
		if (i < 50)
			MaxFP += 6.f;
		else
			MaxFP += 2.f;
	}
	CurrentFP = FMath::Clamp(CurrentFP, 0.f, MaxFP);

	// === Stamina ===
	MaxStamina = BaseStamina;
	for (int32 i = 2; i <= EnduranceLevel; ++i)
	{
		if (i < 30)
			MaxStamina += 2.f;
		else
			MaxStamina += 1.f;
	}
	CurrentStamina = FMath::Clamp(CurrentStamina, 0.f, MaxStamina);

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AALSBaseCharacter* ALSChar = Cast<AALSBaseCharacter>(PC->GetCharacter()))
		{
			if (ALSChar->PlayerHUDWidget)
			{
				ALSChar->PlayerHUDWidget->UpdateRunes(CurrentRunes);
			}
		}
	}

	// ? Broadcast to update HUD
	OnStatsChanged.Broadcast(CurrentHealth, MaxHealth, CurrentFP, MaxFP, CurrentStamina, MaxStamina, CurrentLevel, MaxLevel);
}
