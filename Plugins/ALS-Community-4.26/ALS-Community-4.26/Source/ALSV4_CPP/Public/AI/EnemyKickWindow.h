// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "EnemyKickWindow.generated.h"

UCLASS()
class ALSV4_CPP_API UEnemyKickWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kick")
	float DamageAmount = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kick")
	float HitRadius = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kick")
	float HitForwardOffset = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kick")
	float LaunchStrength = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kick")
	float LaunchUpwardStrength = 180.0f;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	                         const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
	                        const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	                       const FAnimNotifyEventReference& EventReference) override;
};
