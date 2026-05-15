// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"
#include "AI/EnemyHealthBarWidgetComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "EnemyHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDeath);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALSV4_CPP_API UEnemyHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyHealthComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runes")
	int32 RunesToDrop = 100; // Default value; override per-enemy in Blueprint

	UPROPERTY(EditAnywhere, Category = "Runes")
	UNiagaraSystem* RuneDropFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float CurrentHealth;

	UFUNCTION(BlueprintCallable)
	void TakeDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetInvincibleForDuration(float Duration);

	UFUNCTION(BlueprintCallable)
	bool IsDeadOrOutOfHealth() const { return bIsDead || CurrentHealth <= 0.0f; }

	UFUNCTION(BlueprintCallable)
	void ShowHealthBar(bool bShow);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEnemyDeath OnDeath;

	void ClearLockOnIfTargetDies();

	UPROPERTY(BlueprintReadOnly)
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	bool bIsInvincible = false;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UEnemyHealthBarWidgetComponent* HealthBarWidget;

	FTimerHandle InvincibilityTimerHandle;

	void HandleDeath();
	void ResetInvincibility();
	void UpdateHealthBar();
	void CacheHealthBarWidget();
};
