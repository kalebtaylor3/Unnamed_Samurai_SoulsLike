// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UI/DeathScreenWidget.h"

#include "Character/ALSBaseCharacter.h"
#include "Character/PlayerStatsComponent.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void UDeathScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	DeathAnimationTime = 0.0f;
	bDeathAnimationFinished = false;
	bRespawnTriggered = false;

	if (RespawnButton)
	{
		RespawnButton->OnClicked.AddDynamic(this, &UDeathScreenWidget::HandleRespawnClicked);
	}

	UpdateDeadTextAnimation(0.0f);
}

void UDeathScreenWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bDeathAnimationFinished)
	{
		return;
	}

	DeathAnimationTime += InDeltaTime;

	const float Duration = FMath::Max(DeathAnimationDuration, KINDA_SMALL_NUMBER);
	const float NormalizedTime = FMath::Clamp(DeathAnimationTime / Duration, 0.0f, 1.0f);
	UpdateDeadTextAnimation(NormalizedTime);

	if (NormalizedTime >= 1.0f)
	{
		bDeathAnimationFinished = true;

		if (bAutoRespawnWhenAnimationFinishes)
		{
			if (RespawnDelayAfterFadeOut > 0.0f)
			{
				GetWorld()->GetTimerManager().SetTimer(
					RespawnDelayTimerHandle,
					this,
					&UDeathScreenWidget::RespawnAtBonfire,
					RespawnDelayAfterFadeOut,
					false
				);
			}
			else
			{
				RespawnAtBonfire();
			}
		}
	}
}

void UDeathScreenWidget::HandleRespawnClicked()
{
	RespawnAtBonfire();
}

void UDeathScreenWidget::RespawnAtBonfire()
{
	if (bRespawnTriggered)
	{
		return;
	}

	bRespawnTriggered = true;

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	if (AALSBaseCharacter* Player = Cast<AALSBaseCharacter>(PC->GetCharacter()))
	{
		if (UPlayerStatsComponent* Stats = Player->FindComponentByClass<UPlayerStatsComponent>())
		{
			Stats->LoadGameFromBonfire();
		}
	}
}

void UDeathScreenWidget::UpdateDeadTextAnimation(float NormalizedTime)
{
	float BackgroundOpacity = 1.0f;
	if (NormalizedTime < 0.16f)
	{
		BackgroundOpacity = EaseOutCubic(NormalizedTime / 0.16f);
	}
	else if (NormalizedTime > 0.88f)
	{
		BackgroundOpacity = 1.0f - EaseInOutSine((NormalizedTime - 0.88f) / 0.12f);
	}

	if (YouDiedBG)
	{
		YouDiedBG->SetRenderOpacity(FMath::Clamp(BackgroundOpacity, 0.0f, 1.0f));
	}

	if (!DeadText)
	{
		return;
	}

	const float TravelAlpha = EaseInOutSine(NormalizedTime);
	const FVector2D StartOffset(-8.0f, 2.0f);
	const FVector2D EndOffset(8.0f, -2.0f);
	const FVector2D DriftOffset = FMath::Lerp(StartOffset, EndOffset, TravelAlpha);

	const float FloatOffset = FMath::Sin(NormalizedTime * PI * 1.35f) * -2.0f;
	const float SlowLift = FMath::Lerp(2.0f, -3.0f, TravelAlpha);
	DeadText->SetRenderTranslation(DriftOffset + FVector2D(0.0f, FloatOffset + SlowLift));

	const float Scale = FMath::Lerp(0.985f, 1.018f, TravelAlpha);
	DeadText->SetRenderScale(FVector2D(Scale, Scale));

	float TextOpacity = 1.0f;
	if (NormalizedTime < 0.34f)
	{
		TextOpacity = EaseOutCubic(FMath::Max(0.0f, NormalizedTime - 0.12f) / 0.22f);
	}
	else if (NormalizedTime > 0.82f)
	{
		TextOpacity = 1.0f - EaseInOutSine((NormalizedTime - 0.82f) / 0.18f);
	}

	DeadText->SetRenderOpacity(FMath::Clamp(TextOpacity, 0.0f, 1.0f));
}

float UDeathScreenWidget::EaseInOutSine(float Value) const
{
	const float ClampedValue = FMath::Clamp(Value, 0.0f, 1.0f);
	return -(FMath::Cos(PI * ClampedValue) - 1.0f) * 0.5f;
}

float UDeathScreenWidget::EaseOutCubic(float Value) const
{
	const float ClampedValue = FMath::Clamp(Value, 0.0f, 1.0f);
	return 1.0f - FMath::Pow(1.0f - ClampedValue, 3.0f);
}

float UDeathScreenWidget::EaseInCubic(float Value) const
{
	const float ClampedValue = FMath::Clamp(Value, 0.0f, 1.0f);
	return ClampedValue * ClampedValue * ClampedValue;
}
