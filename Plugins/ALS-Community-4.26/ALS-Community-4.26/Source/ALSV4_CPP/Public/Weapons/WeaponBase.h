// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <Library/ALSCharacterEnumLibrary.h>
#include "Weapons/HeldWeaponBase.h"
#include "Weapons/AshOfWarBase.h"
#include "WeaponBase.generated.h"

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AshOfWar")
	TSubclassOf<UAshOfWarBase> AshOfWarClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	UTexture2D* AshOfWarIcon;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	FName WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	EALSOverlayState OverlayType;
};