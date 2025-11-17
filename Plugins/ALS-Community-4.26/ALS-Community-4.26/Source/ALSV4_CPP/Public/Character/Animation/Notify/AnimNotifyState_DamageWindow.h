// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Character/ALSBaseCharacter.h"
#include "AnimNotifyState_DamageWindow.generated.h"


UCLASS()
class ALSV4_CPP_API UAnimNotifyState_DamageWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	// Damage defined per notify window
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float DamageAmount = 25.0f;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};