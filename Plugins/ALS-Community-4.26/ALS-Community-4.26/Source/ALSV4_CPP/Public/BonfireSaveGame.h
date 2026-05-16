// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Weapons/WeaponBase.h"
#include "Weapons/SpellBase.h"
#include "BonfireSaveGame.generated.h"

USTRUCT()
struct FEnemySaveData
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FName EnemyID;

	UPROPERTY()
	bool bIsDead;
};

/**
 * 
 */
UCLASS()
class ALSV4_CPP_API UBonfireSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FVector PlayerLocation;

	UPROPERTY()
	FRotator PlayerRotation;

	UPROPERTY()
	float Health;

	UPROPERTY()
	float MaxHealth;

	UPROPERTY()
	int32 CurrentRunes;

	UPROPERTY()
	TArray<FEnemySaveData> EnemyStates;

	UPROPERTY(BlueprintReadWrite)
	int32 PlayerLevel;

	UPROPERTY(BlueprintReadWrite)
	int32 VigorLevel;

	UPROPERTY(BlueprintReadWrite)
	int32 MindLevel;

	UPROPERTY(BlueprintReadWrite)
	int32 EnduranceLevel;

	UPROPERTY()
	FString LevelName;

	// Inventory weapons
	UPROPERTY()
	TArray<TSubclassOf<UWeaponBase>> SavedEquippedWeapons;

	UPROPERTY()
	TArray<TSubclassOf<UWeaponBase>> SavedBackpackWeapons;

	UPROPERTY(BlueprintReadWrite)
	int32 SavedEquippedIndex = -1;

	// Inventory spells
	UPROPERTY()
	TArray<TSubclassOf<USpellBase>> SavedEquippedSpells;

	UPROPERTY()
	TArray<TSubclassOf<USpellBase>> SavedBackpackSpells;

	UPROPERTY(BlueprintReadWrite)
	int32 SavedEquippedSpellIndex = -1;

	UPROPERTY(BlueprintReadWrite)
	bool bHasRuneDrop = false;

	UPROPERTY(BlueprintReadWrite)
	int32 DroppedRuneAmount = 0;

	UPROPERTY(BlueprintReadWrite)
	FVector DroppedRuneLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	TArray<FName> CollectedLootIDs;
};
