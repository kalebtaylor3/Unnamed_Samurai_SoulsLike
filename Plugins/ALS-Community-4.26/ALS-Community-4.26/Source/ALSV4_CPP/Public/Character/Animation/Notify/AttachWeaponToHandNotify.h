// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AttachWeaponToHandNotify.generated.h"

UCLASS()
class ALSV4_CPP_API UAttachWeaponToHandNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attach")
	bool bLeftHand = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attach")
	bool bUseCurrentWeaponPlacement = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attach", meta = (EditCondition = "!bUseCurrentWeaponPlacement"))
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attach", meta = (EditCondition = "!bUseCurrentWeaponPlacement"))
	FRotator RotationOffset = FRotator::ZeroRotator;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	virtual FString GetNotifyName_Implementation() const override;
};
