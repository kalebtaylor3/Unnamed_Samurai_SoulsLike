// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/CombatComponent.h"
#include "PlayerStatsComponent.generated.h"

class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_EightParams(FOnStatsChanged, float, CurrentHealth, float, MaxHealth, float, CurrentFP, float, MaxFP, float, CurrentStamina, float, MaxStamina, int32, CurrentLevel, int32, MaxLevel);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRunesChanged, int32, CurrentRunes, int32, RunesNeeded);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALSV4_CPP_API UPlayerStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerStatsComponent();

protected:
	virtual void BeginPlay() override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 CurrentLevel = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	int32 MaxLevel = 150; // or whatever you define as max

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 VigorLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 MindLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 EnduranceLevel = 1;

	/** Max and current values */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 448.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float CurrentHealth = 448.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxFP = 62.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float CurrentFP = 62.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxStamina = 50.f;

	UPROPERTY(EditAnywhere, Category = "Stats")
	float CurrentStamina = 50.f;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDied OnPlayerDied;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	bool bIsDead = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* HitReactMontage;

	UPROPERTY()
	UCombatComponent* CombatComponent;

	/** Called when any stat is updated */
	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnStatsChanged OnStatsChanged;

	FTimerHandle HealthRegenHandle;
	float RemainingHealAmount = 0.f;
	float HealTickAmount = 0.f;
	float HealTickRate = 0.05f; // every 0.05s (20 times per second)

	FTimerHandle FPRegenHandle;
	float RemainingFPAmount = 0.f;
	float FPTickAmount = 0.f;
	float FPTickRate = 0.05f; // Tick every 0.05s like health

	UPROPERTY(BlueprintReadOnly)
	bool bBeingHit = false;

	// Timer handle to reset bBeingHit
	FTimerHandle BeingHitResetTimer;

	UPROPERTY(BlueprintReadOnly)
	bool bIsInvincible = false;

	FTimerHandle IFrameResetHandle;

	void ResetInvincibility();

	// Function to reset the being hit flag
	void ResetBeingHitFlag();

	void ApplyHealthTick();

	/** Use FP or Stamina */
	UFUNCTION(BlueprintCallable)
	void UseFP(float Amount);

	UFUNCTION(BlueprintCallable)
	void UseStamina(float Amount);

	UFUNCTION(BlueprintCallable)
	void NotifyStaminaExhausted();

	UPROPERTY(BlueprintReadWrite, Category = "Regen")
	bool bCanRegenStamina = true;

	FTimerHandle StaminaRegenHandle;
	FTimerHandle StaminaRegenDelayHandle;

	float RemainingStaminaToRegen = 0.f;
	float StaminaRegenPerTick = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Regen")
	float StaminaRegenTickRate = 0.05f; // Every 0.05s

	void RegenerateStamina(float Amount);
	void ApplyStaminaRegenTick();

	UFUNCTION(BlueprintCallable)
	void TakeDamage(float Amount);

	UFUNCTION(BlueprintCallable)
	void RestoreHealth(float Amount);

	UFUNCTION(BlueprintCallable)
	void RestoreFP(float Amount);

	void ApplyFPTick();

	/** Called on level up to expand bars */
	UFUNCTION(BlueprintCallable)
	void SetMaxValues(float NewHealth, float NewFP, float NewStamina);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void IncreaseLevel(int32 HealthLevels, int32 FPLevels, int32 StaminaLevels);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runes")
	int32 CurrentRunes = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runes")
	int32 RunesNeeded = 0;

	UPROPERTY(BlueprintAssignable, Category = "Runes")
	FOnRunesChanged OnRunesChanged;

	UFUNCTION(BlueprintCallable, Category = "Runes")
	int32 GetRunesRequiredForLevel(int32 Level) const;

	UFUNCTION(BlueprintCallable, Category = "Runes")
	bool TrySpendRunes(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Runes")
	void AddRunes(int32 Amount);

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> DeathScreenClass;

	UPROPERTY()
	UUserWidget* ActiveDeathScreen;

	void HandlePlayerDeath();

	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void LoadGameFromBonfire();

	UFUNCTION(BlueprintCallable)
	void RecalculateDerivedStats();

};
