// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include <Components/Image.h>
#include <Components/TextBlock.h>
#include <Components/ProgressBar.h>
#include <Components/BorderSlot.h>
#include "Components/Border.h"
#include "PaperSprite.h"
#include "Character/UI/DragItemIconWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "PlayerHUDUI.generated.h"


/**
 * 
 */
UCLASS()
class ALSV4_CPP_API UPlayerHUDUI : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Images")
	UPaperSprite* DefaultSlotBGSprite;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Images")
	UPaperSprite* HighlightedSlotBGSprite;

	/** Call this from inventory/cycling code to update the weapon icon */
	UFUNCTION(BlueprintCallable)
	void UpdateWeaponIcon(UTexture2D* NewIcon);

	UFUNCTION(BlueprintCallable)
	void UpdateWeaponIconSprite(UPaperSprite* NewIcon);

	void NativeConstruct();

	UFUNCTION(BlueprintCallable)
	void UpdateAshOfWarIcon(UTexture2D* NewIcon);

	UFUNCTION(BlueprintCallable)
	void UpdateAshOfWarIconSprite(UPaperSprite* NewIcon);

	UFUNCTION(BlueprintCallable)
	void UpdatePotionIcon(UTexture2D* NewIcon);

	UFUNCTION(BlueprintCallable)
	void UpdatePotionIconSprite(UPaperSprite* NewIcon);

	UFUNCTION(BlueprintCallable)
	void UpdatePotionCount(int32 NewCount);

	UFUNCTION(BlueprintCallable)
	void SetStats(float CurrentHealth, float MaxHealth, float CurrentFP, float MaxFP, float CurrentStamina, float MaxStamina, int32 CurrentLevel, int32 MaxLevel);

	UFUNCTION(BlueprintCallable)
	void UpdateInventorySlot(int32 SlotIndex, UTexture2D* Icon);

	UFUNCTION(BlueprintCallable)
	void UpdateInventorySlotSprite(int32 SlotIndex, UPaperSprite* Icon);

	UFUNCTION(BlueprintCallable)
	void HighlightSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable)
	void ClearAllHighlights();

	void SetSlotToDefaultColor(int32 SlotIndex);

	int32 CurrentSwapSlot = -1;
	int32 HoveredSlotIndex = -1;

	UFUNCTION(BlueprintCallable)
	void UpdateRunes(int32 NewRunes);

	UFUNCTION(BlueprintCallable)
	void SetInventoryPanelVisible(bool bVisible);

	int32 GetHoveredSlotIndex() const { return HoveredSlotIndex; }
	bool IsMouseHoveringSlot() const { return HoveredSlotIndex != -1; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> DragIconClass;

	UPROPERTY()
	UDragItemIconWidget* DragIconWidget;

	void ShowDragIcon(UTexture2D* Icon);
	void ShowDragIconSprite(UPaperSprite* Icon);
	void HideDragIcon();

protected:

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* SafeArea_16_9;

	// Helper to recompute the safe area rectangle
	void UpdateSafeArea();

	// Cache last viewport size so we only update when needed
	FIntPoint CachedViewportSize = FIntPoint::ZeroValue;

	/** This will be bound in the UMG editor using the same name */
	UPROPERTY(meta = (BindWidget))
	UImage* WeaponSlotIcon;

	UPROPERTY(meta = (BindWidget))
	UImage* AshOfWarSlotIcon;

	UPROPERTY(meta = (BindWidget))
	UImage* PotionsSlotIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PotionsSlotCount;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HPBar;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HPBar_Back;

	FTimerHandle BackBarUpdateHandle;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* FPBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* StaminaBar;

	FLinearColor DefaultSlotColor = FLinearColor::White;
	FLinearColor HighlightedSlotColor = FLinearColor::Yellow;
	FLinearColor SwapSelectedSlotColor = FLinearColor::Yellow;

	/// <summary>
	/// Inventory
	/// </summary>

	UPROPERTY(meta = (BindWidgetOptional))
	UCanvasPanel* InventoryPannel;

	UPROPERTY(meta = (BindWidget))
	UImage* WeaponSlotImage_0;

	UPROPERTY(meta = (BindWidget))
	UImage* WeaponSlotImage_1;

	UPROPERTY(meta = (BindWidget))
	UImage* WeaponSlotImage_2;

	UPROPERTY(meta = (BindWidget))
	UImage* WeaponSlotImage_3;

	UPROPERTY(meta = (BindWidget)) 
	UBorder* WeaponSlotBG_0;
	UPROPERTY(meta = (BindWidget))
	UBorder* WeaponSlotBG_1;
	UPROPERTY(meta = (BindWidget))
	UBorder* WeaponSlotBG_2;
	UPROPERTY(meta = (BindWidget))
	UBorder* WeaponSlotBG_3;

	UPROPERTY(meta = (BindWidget))
	UImage* InventorySlotImage_0;

	UPROPERTY(meta = (BindWidget))
	UImage* InventorySlotImage_1;

	UPROPERTY(meta = (BindWidget))
	UImage* InventorySlotImage_2;

	UPROPERTY(meta = (BindWidget))
	UImage* InventorySlotImage_3;

	UPROPERTY(meta = (BindWidget))
	UImage* InventorySlotImage_4;

	UPROPERTY(meta = (BindWidget))
	UImage* InventorySlotImage_5;

	UPROPERTY(meta = (BindWidget))
	UImage* InventorySlotImage_6;

	UPROPERTY(meta = (BindWidget))
	UImage* InventorySlotImage_7;

	UPROPERTY(meta = (BindWidget))
	UImage* InventorySlotImage_8;

	UPROPERTY(meta = (BindWidget))
	UImage* InventorySlotImage_9;

	UPROPERTY(meta = (BindWidget))
	UBorder* InventorySlotBG_0;
	UPROPERTY(meta = (BindWidget))
	UBorder* InventorySlotBG_1;
	UPROPERTY(meta = (BindWidget))
	UBorder* InventorySlotBG_2;
	UPROPERTY(meta = (BindWidget))
	UBorder* InventorySlotBG_3;
	UPROPERTY(meta = (BindWidget))
	UBorder* InventorySlotBG_4;
	UPROPERTY(meta = (BindWidget))
	UBorder* InventorySlotBG_5;
	UPROPERTY(meta = (BindWidget))
	UBorder* InventorySlotBG_6;
	UPROPERTY(meta = (BindWidget))
	UBorder* InventorySlotBG_7;
	UPROPERTY(meta = (BindWidget))
	UBorder* InventorySlotBG_8;
	UPROPERTY(meta = (BindWidget))
	UBorder* InventorySlotBG_9;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentRunes;

	UPROPERTY()
	TArray<UWidget*> AllSlotWidgets; // Filled in Construct or Init

	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime);
};
