// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WBP_EnemyHealthBar.generated.h"


UCLASS()
class ALSV4_CPP_API UWBP_EnemyHealthBar : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetHealthPercent(float InPercent);

protected:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar_Front; // Actual health (white)

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar_Back;  // Delayed damage (red)

	FTimerHandle BackBarTimerHandle;
	float CurrentBackPercent = 1.f;
};