// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/AshOfWarBase.h"
#include "NiagaraSystem.h"
#include "BuffAshOfWar.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, EditInlineNew)
class ALSV4_CPP_API UBuffAshOfWar : public UAshOfWarBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buff")
	UAnimMontage* BuffMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buff")
	UNiagaraSystem* BuffEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buff")
	float BuffDuration = 10.f; // seconds

	/** Called when player presses L2 or similar input */
	virtual void ActivateAsh_Implementation(AALSBaseCharacter* Character) override;

	/** Called when animation notify "Buff" triggers */
	virtual void OnBuffNotify(AALSBaseCharacter* Character);
	
};
