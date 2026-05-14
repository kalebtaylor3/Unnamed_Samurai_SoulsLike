// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <Library/ALSCharacterEnumLibrary.h>
#include "Weapons/HeldWeaponBase.h"
#include "Weapons/AshOfWarBase.h"
#include "PaperSprite.h"
#include "WeaponBase.generated.h"

class AWeaponArrowProjectile;
class UCameraShakeBase;

UCLASS(Blueprintable, BlueprintType)
class ALSV4_CPP_API UWeaponBase : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	UStaticMesh* WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	TSubclassOf<class AHeldWeaponBase> HeldActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	UTexture2D* WeaponIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	UPaperSprite* WeaponIconSprite;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AshOfWar")
	TSubclassOf<UAshOfWarBase> AshOfWarClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	UTexture2D* AshOfWarIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	UPaperSprite* AshOfWarIconSprite;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float LightAttackStaminaAmount = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float HeavyAttackStaminaAmount = 40;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float JumpAttackStaminaAmount = 25;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FVector PlacementPosition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FRotator PlacementRotation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TArray<UAnimMontage*> OneHandedLightAttackMontages;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TArray<UAnimMontage*> TwoHandedLightAttackMontages;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* OneHJumpAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* TwoHJumpAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* ChargeMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* ChargeMontageLoop;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* HeavyAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow")
	bool bIsBow = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (EditCondition = "bIsBow"))
	TSubclassOf<AWeaponArrowProjectile> ArrowProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (EditCondition = "bIsBow"))
	UAnimMontage* BowDrawMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (EditCondition = "bIsBow"))
	UAnimMontage* BowDrawLoopMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (EditCondition = "bIsBow"))
	UAnimMontage* BowFireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (EditCondition = "bIsBow"))
	TSubclassOf<UCameraShakeBase> BowFireCameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (EditCondition = "bIsBow"))
	float BowFireCameraShakeScale = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (EditCondition = "bIsBow"))
	FName ArrowSpawnSocketName = TEXT("ArrowSocket");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (EditCondition = "bIsBow"))
	UStaticMesh* PreviewArrowMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (EditCondition = "bIsBow"))
	FVector PreviewArrowLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (EditCondition = "bIsBow"))
	FRotator PreviewArrowRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (EditCondition = "bIsBow"))
	FVector PreviewArrowScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (EditCondition = "bIsBow"))
	FVector ArrowSpawnOffset = FVector(60.0f, 0.0f, 40.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (EditCondition = "bIsBow"))
	float ArrowDamage = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (EditCondition = "bIsBow"))
	float ArrowSpeed = 4500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (EditCondition = "bIsBow"))
	float AimTraceRange = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	FName WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	EALSOverlayState OverlayType;
};
