// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapons/WeaponBase.h"
#include "Weapons/SpellBase.h"
#include "Character/UI/DragItemIconWidget.h"
#include "InventoryComponent.generated.h"

class UPaperSprite;


UENUM(BlueprintType)
enum class EPotionType : uint8
{
	HP,
	FP
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ALSV4_CPP_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	void UpdateInventoryUI();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons")
	TArray<TSubclassOf<UWeaponBase>> BackpackWeapons;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons")
	TArray<TSubclassOf<UWeaponBase>> EquippedWeapons;

	UPROPERTY(BlueprintReadOnly, Category = "Weapons")
	UWeaponBase* CurrentWeapon;

	UPROPERTY(BlueprintReadOnly, Category = "Weapons")
	int32 EquippedIndex = -1;

	// Eventually support two-handing, magic offhands, etc.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void EquipWeaponByIndex(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CycleNextWeapon();  // D-pad right

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RefreshEquippedWeaponsFromBackpack();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spells")
	TArray<TSubclassOf<USpellBase>> BackpackSpells;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spells")
	TArray<TSubclassOf<USpellBase>> EquippedSpells;

	UPROPERTY(BlueprintReadOnly, Category = "Spells")
	USpellBase* CurrentSpell;

	UPROPERTY(BlueprintReadOnly, Category = "Spells")
	int32 EquippedSpellIndex = -1;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void EquipSpellByIndex(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CycleNextSpell();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RefreshEquippedSpellsFromBackpack();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	USpellBase* GetEquippedSpell() const;

	FTimerHandle SwapCooldownTimer;
	bool bSwapCooldownActive = false;

	UPROPERTY()
	bool bCanCycleWeapon = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potions")
	int32 MaxHPPotions = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potions")
	int32 MaxFPPotions = 2;

	UPROPERTY(BlueprintReadOnly, Category = "Potions")
	int32 CurrentHPPotions = 3;

	UPROPERTY(BlueprintReadOnly, Category = "Potions")
	int32 CurrentFPPotions = 2;


	UPROPERTY(BlueprintReadOnly, Category = "Potions")
	EPotionType ActivePotion = EPotionType::HP;

	UPROPERTY(EditAnywhere, Category = "Potions")
	UTexture2D* HPPotionIcon;

	UPROPERTY(EditAnywhere, Category = "Potions")
	UPaperSprite* HPPotionIconSprite;

	UPROPERTY(EditAnywhere, Category = "Potions")
	UTexture2D* FPPotionIcon;

	UPROPERTY(EditAnywhere, Category = "Potions")
	UPaperSprite* FPPotionIconSprite;


	UFUNCTION(BlueprintCallable)
	void UseActivePotion();

	UFUNCTION(BlueprintCallable)
	void TogglePotionType();  // D-pad Down

	void UpdatePotionHUD();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Healing")
	bool bHealing = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* HPHealMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* FPHealMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	UTexture2D* EmptySlotIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	UPaperSprite* EmptySlotIconSprite;

	UPROPERTY(BlueprintReadOnly)
	int32 SelectedSlotIndex = -1;

	UPROPERTY(BlueprintReadOnly)
	bool bIsInventoryOpen = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsSwapping = false;

	void SelectSlot(int32 Index);
	void ClearSelection();
	void SwapWeapons(int32 SlotA, int32 SlotB);

	void SaveInventory();

	UPROPERTY()
	UDragItemIconWidget* DragIconWidget = nullptr;

	UTexture2D* GetIconAtIndex(int32 Index) const;
	UPaperSprite* GetIconSpriteAtIndex(int32 Index) const;
		
};
