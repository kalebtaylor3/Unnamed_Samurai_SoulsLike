// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeathScreenWidget.generated.h"

class UButton;
class UBorder;
class UTextBlock;

UCLASS()
class ALSV4_CPP_API UDeathScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Death")
	void RespawnAtBonfire();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* RespawnButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* DeadText;

	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* YouDiedBG;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Death Animation")
	float DeathAnimationDuration = 4.85f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Death Animation")
	bool bAutoRespawnWhenAnimationFinishes = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Death Animation")
	float RespawnDelayAfterFadeOut = 0.35f;

	UFUNCTION()
	void HandleRespawnClicked();

private:
	float DeathAnimationTime = 0.0f;
	bool bDeathAnimationFinished = false;
	bool bRespawnTriggered = false;
	FTimerHandle RespawnDelayTimerHandle;

	void UpdateDeadTextAnimation(float NormalizedTime);
	float EaseInOutSine(float Value) const;
	float EaseOutCubic(float Value) const;
	float EaseInCubic(float Value) const;
};
