// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Weapons/WeaponBase.h"
#include "MyGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class ALSV4_CPP_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	bool bShouldRestoreFromBonfire = false;

	UPROPERTY(BlueprintReadWrite)
	FVector SavedBonfireLocation;

	UPROPERTY(BlueprintReadWrite)
	FRotator SavedBonfireRotation;

	UPROPERTY(BlueprintReadWrite)
	float SavedHealth = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float SavedMaxHealth = 0.f;

	UPROPERTY(BlueprintReadWrite)
	int32 SavedRunes = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 SavedLevel = 1;

	UPROPERTY(BlueprintReadWrite)
	int32 SavedVigor = 1;

	UPROPERTY(BlueprintReadWrite)
	int32 SavedMind = 1;

	UPROPERTY(BlueprintReadWrite)
	int32 SavedEndurance = 1;

	UPROPERTY(BlueprintReadWrite)
	FString SavedLevelName;

	UPROPERTY(BlueprintReadWrite)
	TArray<TSubclassOf<UWeaponBase>> SavedBackpackWeapons;

	UPROPERTY(BlueprintReadWrite)
	TArray<TSubclassOf<UWeaponBase>> SavedEquippedWeapons;

	UPROPERTY(BlueprintReadWrite)
	int32 SavedEquippedIndex = -1;

	UPROPERTY(BlueprintReadWrite, Category = "Bonfire")
	bool bWasKilled = false;

	UPROPERTY(BlueprintReadWrite)
	bool bHasRuneDrop = false;

	UPROPERTY(BlueprintReadWrite)
	FVector DroppedRuneLocation;

	UPROPERTY(BlueprintReadWrite)
	int32 DroppedRuneAmount = 0;
	
};
