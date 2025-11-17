// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/WBP_EnemyHealthBar.h"
#include "Components/ProgressBar.h"
#include "TimerManager.h"

void UWBP_EnemyHealthBar::SetHealthPercent(float NewPercent)
{
	if (HealthBar_Front)
	{
		HealthBar_Front->SetPercent(NewPercent); // Instant update
	}

	// Animate red bar (back) to slowly match new percent
	if (HealthBar_Back)
	{
		GetWorld()->GetTimerManager().ClearTimer(BackBarTimerHandle);

		// Start interpolation toward the new value
		GetWorld()->GetTimerManager().SetTimer(BackBarTimerHandle, [this, NewPercent]()
			{
				const float InterpSpeed = 1.2f; // Lower = slower delay
				CurrentBackPercent = FMath::FInterpTo(CurrentBackPercent, NewPercent, GetWorld()->GetDeltaSeconds(), InterpSpeed);

				HealthBar_Back->SetPercent(CurrentBackPercent);

				if (FMath::IsNearlyEqual(CurrentBackPercent, NewPercent, 0.01f))
				{
					CurrentBackPercent = NewPercent;
					HealthBar_Back->SetPercent(CurrentBackPercent);
					GetWorld()->GetTimerManager().ClearTimer(BackBarTimerHandle);
				}
			}, 0.01f, true);
	}
}