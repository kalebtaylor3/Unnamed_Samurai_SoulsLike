// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UI/PlayerHUDUI.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Slate/WidgetTransform.h"

namespace
{
	void SetImageFromSprite(UImage* Image, UPaperSprite* Sprite)
	{
		if (!Image)
		{
			return;
		}

		if (Sprite)
		{
			FSlateBrush Brush;
			Brush.SetResourceObject(Sprite);
			Brush.ImageSize = FVector2D(64.f, 64.f);
			Image->SetBrush(Brush);
			Image->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Image->SetBrushFromTexture(nullptr);
			Image->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UPlayerHUDUI::NativeConstruct()
{
	Super::NativeConstruct();

	// Clear in case it's being rebuilt
	AllSlotWidgets.Empty();

	// Add weapon slot borders
	AllSlotWidgets.Add(WeaponSlotBG_0);
	AllSlotWidgets.Add(WeaponSlotBG_1);
	AllSlotWidgets.Add(WeaponSlotBG_2);
	AllSlotWidgets.Add(WeaponSlotBG_3);

	// Add inventory slot borders
	AllSlotWidgets.Add(InventorySlotBG_0);
	AllSlotWidgets.Add(InventorySlotBG_1);
	AllSlotWidgets.Add(InventorySlotBG_2);
	AllSlotWidgets.Add(InventorySlotBG_3);
	AllSlotWidgets.Add(InventorySlotBG_4);
	AllSlotWidgets.Add(InventorySlotBG_5);
	AllSlotWidgets.Add(InventorySlotBG_6);
	AllSlotWidgets.Add(InventorySlotBG_7);
	AllSlotWidgets.Add(InventorySlotBG_8);
	AllSlotWidgets.Add(InventorySlotBG_9);

	if (DragIconClass)
	{
		DragIconWidget = CreateWidget<UDragItemIconWidget>(GetWorld(), DragIconClass);
		if (DragIconWidget)
		{
			DragIconWidget->AddToViewport(999); // High Z-order
			DragIconWidget->SetVisibility(ESlateVisibility::Hidden);
			UE_LOG(LogTemp, Warning, TEXT("Drag icon widget created and added to viewport."));
		}
	}

	UpdateSafeArea();
}

void UPlayerHUDUI::UpdateSafeArea()
{
	if (!SafeArea_16_9)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	int32 ViewX = 0, ViewY = 0;
	PC->GetViewportSize(ViewX, ViewY);
	if (ViewX <= 0 || ViewY <= 0)
	{
		return;
	}

	CachedViewportSize = FIntPoint(ViewX, ViewY);

	// === 1) 16:9 camera rect in *pixels* inside the viewport ===
	const float DesiredAspect = 16.0f / 9.0f;
	const float ViewAspect    = static_cast<float>(ViewX) / static_cast<float>(ViewY);

	float WorldWidthPx  = 0.f;
	float WorldHeightPx = 0.f;
	float WorldXPx      = 0.f;
	float WorldYPx      = 0.f;

	if (ViewAspect > DesiredAspect)
	{
		// Wider than 16:9 -> pillarbox (black bars left/right)
		WorldHeightPx = static_cast<float>(ViewY);
		WorldWidthPx  = WorldHeightPx * DesiredAspect;
		WorldXPx      = 0.5f * (static_cast<float>(ViewX) - WorldWidthPx);
		WorldYPx      = 0.f;
	}
	else
	{
		// Taller than 16:9 -> letterbox (bars top/bottom)
		WorldWidthPx  = static_cast<float>(ViewX);
		WorldHeightPx = WorldWidthPx / DesiredAspect;
		WorldXPx      = 0.f;
		WorldYPx      = 0.5f * (static_cast<float>(ViewY) - WorldHeightPx);
	}

	// === 2) Convert pixel rect -> UMG coordinates (DPI scale) ===
	float DPIScale = UWidgetLayoutLibrary::GetViewportScale(this);
	if (DPIScale <= 0.f)
	{
		DPIScale = 1.f;
	}

	const float WorldWidthUMG  = WorldWidthPx  / DPIScale;
	const float WorldHeightUMG = WorldHeightPx / DPIScale;
	const float WorldXUMG      = WorldXPx      / DPIScale;
	const float WorldYUMG      = WorldYPx      / DPIScale;

	// === 3) Apply to SafeArea_16_9 slot (no extra RenderScale) ===
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(SafeArea_16_9->Slot))
	{
		CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
		CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));
		CanvasSlot->SetPosition(FVector2D(WorldXUMG, WorldYUMG));
		CanvasSlot->SetSize(FVector2D(WorldWidthUMG, WorldHeightUMG));
	}

	// Make sure we are not additionally scaling it
	SafeArea_16_9->SetRenderTransformPivot(FVector2D(0.f, 0.f));
	SafeArea_16_9->SetRenderScale(FVector2D(1.f, 1.f));
}



void UPlayerHUDUI::UpdateWeaponIcon(UTexture2D* NewIcon)
{
	if (!WeaponSlotIcon) return;

	if (NewIcon)
	{
		WeaponSlotIcon->SetBrushFromTexture(NewIcon);
		WeaponSlotIcon->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		WeaponSlotIcon->SetBrushFromTexture(nullptr);
		WeaponSlotIcon->SetVisibility(ESlateVisibility::Hidden); // or Collapsed if you want to remove layout space
	}
}

void UPlayerHUDUI::UpdateWeaponIconSprite(UPaperSprite* NewIcon)
{
	SetImageFromSprite(WeaponSlotIcon, NewIcon);
}

void UPlayerHUDUI::UpdateAshOfWarIcon(UTexture2D* NewIcon)
{
	if (!AshOfWarSlotIcon) return;

	if (NewIcon)
	{
		AshOfWarSlotIcon->SetBrushFromTexture(NewIcon);
		AshOfWarSlotIcon->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		AshOfWarSlotIcon->SetBrushFromTexture(nullptr);
		AshOfWarSlotIcon->SetVisibility(ESlateVisibility::Hidden); // or Collapsed if you want to remove layout space
	}
}

void UPlayerHUDUI::UpdateAshOfWarIconSprite(UPaperSprite* NewIcon)
{
	SetImageFromSprite(AshOfWarSlotIcon, NewIcon);
}

void UPlayerHUDUI::UpdatePotionIcon(UTexture2D* NewIcon)
{
	if (PotionsSlotIcon)
	{
		PotionsSlotIcon->SetBrushFromTexture(NewIcon);
		PotionsSlotIcon->SetVisibility(NewIcon ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UPlayerHUDUI::UpdatePotionIconSprite(UPaperSprite* NewIcon)
{
	SetImageFromSprite(PotionsSlotIcon, NewIcon);
}

void UPlayerHUDUI::UpdatePotionCount(int32 NewCount)
{
	if (PotionsSlotCount)
	{
		PotionsSlotCount->SetText(FText::AsNumber(NewCount));
	}
}

void UPlayerHUDUI::SetStats(float CurrentHealth, float MaxHealth, float CurrentFP, float MaxFP, float CurrentStamina, float MaxStamina, int32 CurrentLevel, int32 MaxLevel)
{
	// Calculate percents
	const float HealthPercent = CurrentHealth / MaxHealth;
	const float FPPercent = CurrentFP / MaxFP;
	const float StaminaPercent = CurrentStamina / MaxStamina;

	// === FRONT BARS ===
	HPBar->SetPercent(HealthPercent);
	FPBar->SetPercent(FPPercent);
	StaminaBar->SetPercent(StaminaPercent);

	// === BACK HP BAR SMOOTHING ===
	const float CurrentBackPercent = HPBar_Back ? HPBar_Back->GetPercent() : 1.0f;

	if (HPBar_Back)
	{
		if (HealthPercent < CurrentBackPercent)
		{
			// Clear any old timer
			GetWorld()->GetTimerManager().ClearTimer(BackBarUpdateHandle);

			// Smooth update to back bar
			GetWorld()->GetTimerManager().SetTimer(BackBarUpdateHandle, [this, HealthPercent]()
				{
					if (!HPBar_Back) return;

					const float CurrentPercent = HPBar_Back->GetPercent();
					const float LerpSpeed = 0.05f;
					const float NewPercent = FMath::FInterpTo(CurrentPercent, HealthPercent, GetWorld()->DeltaTimeSeconds, 3.0f);
					HPBar_Back->SetPercent(NewPercent);

					// Stop when close enough
					if (FMath::IsNearlyEqual(NewPercent, HealthPercent, 0.01f))
					{
						HPBar_Back->SetPercent(HealthPercent);
						GetWorld()->GetTimerManager().ClearTimer(BackBarUpdateHandle);
					}

				}, 0.02f, true);
		}
		else
		{
			// HP increased � instantly sync back bar
			HPBar_Back->SetPercent(HealthPercent);
		}
	}

	// === RESIZE BASED ON LEVEL PROGRESSION ===

	// Full length in widget blueprint at max level
	const float MaxHPBarVisualWidth = 909.f;
	const float MaxFPBarVisualWidth = 804.f;
	const float MaxStaminaBarVisualWidth = 666.f;

	// Final max stat values
	const float FinalMaxHealth = 3369.f;
	const float FinalMaxFP = 450.f;
	const float FinalMaxStamina = 170.f;

	const float HealthRatio = FMath::Clamp(MaxHealth / FinalMaxHealth, 0.0f, 1.0f);
	const float FPRatio = FMath::Clamp(MaxFP / FinalMaxFP, 0.0f, 1.0f);
	const float StaminaRatio = FMath::Clamp(MaxStamina / FinalMaxStamina, 0.0f, 1.0f);

	const float HPScaleX = HealthRatio;
	const float FPScaleX = FPRatio * (MaxFPBarVisualWidth / MaxHPBarVisualWidth);
	const float StaminaScaleX = StaminaRatio * (MaxStaminaBarVisualWidth / MaxHPBarVisualWidth);

	HPBar->SetRenderScale(FVector2D(HPScaleX, 1.0f));
	FPBar->SetRenderScale(FVector2D(FPScaleX, 1.0f));
	StaminaBar->SetRenderScale(FVector2D(StaminaScaleX, 1.0f));
	if (HPBar_Back)
	{
		HPBar_Back->SetRenderScale(FVector2D(HPScaleX, 1.0f));
		HPBar_Back->SetRenderTransformPivot(FVector2D(0.0f, 0.5f));
	}

	// Ensure scaling happens from the left
	HPBar->SetRenderTransformPivot(FVector2D(0.0f, 0.5f));
	FPBar->SetRenderTransformPivot(FVector2D(0.0f, 0.5f));
	StaminaBar->SetRenderTransformPivot(FVector2D(0.0f, 0.5f));
}

void UPlayerHUDUI::PlayStaminaDeniedFeedback()
{
	if (!StaminaBar || !GetWorld())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(StaminaDeniedFeedbackHandle);
	StaminaDeniedFeedbackStep = 0;

	if (!bCachedStaminaBarStyle)
	{
		CachedStaminaBarStyle = StaminaBar->GetWidgetStyle();
		bCachedStaminaBarStyle = true;
	}

	FProgressBarStyle DeniedStyle = CachedStaminaBarStyle;
	DeniedStyle.BackgroundImage.TintColor = FSlateColor(DeniedStaminaBarBackgroundTint);
	StaminaBar->SetWidgetStyle(DeniedStyle);

	GetWorld()->GetTimerManager().SetTimer(StaminaDeniedFeedbackHandle, [this]()
		{
			if (!StaminaBar)
			{
				GetWorld()->GetTimerManager().ClearTimer(StaminaDeniedFeedbackHandle);
				return;
			}

			constexpr int32 MaxSteps = 8;
			const float ShakeOffset = (StaminaDeniedFeedbackStep % 2 == 0) ? 7.0f : -7.0f;
			StaminaBar->SetRenderTranslation(FVector2D(ShakeOffset, 0.0f));

			++StaminaDeniedFeedbackStep;
			if (StaminaDeniedFeedbackStep >= MaxSteps)
			{
				StaminaBar->SetRenderTranslation(FVector2D::ZeroVector);
				StaminaBar->SetWidgetStyle(CachedStaminaBarStyle);
				GetWorld()->GetTimerManager().ClearTimer(StaminaDeniedFeedbackHandle);
			}
		}, 0.035f, true);
}

void UPlayerHUDUI::UpdateInventorySlot(int32 SlotIndex, UTexture2D* Icon)
{
	UImage* SlotImage = nullptr;

	switch (SlotIndex)
	{
	case 0: SlotImage = InventorySlotImage_0; break;
	case 1: SlotImage = InventorySlotImage_1; break;
	case 2: SlotImage = InventorySlotImage_2; break;
	case 3: SlotImage = InventorySlotImage_3; break;
	case 4: SlotImage = InventorySlotImage_4; break;
	case 5: SlotImage = InventorySlotImage_5; break;
	case 6: SlotImage = InventorySlotImage_6; break;
	case 7: SlotImage = InventorySlotImage_7; break;
	case 8: SlotImage = InventorySlotImage_8; break;
	case 9: SlotImage = InventorySlotImage_9; break;

		// Equipped slots: use special index range 100+
	case 100: SlotImage = WeaponSlotImage_0; break;
	case 101: SlotImage = WeaponSlotImage_1; break;
	case 102: SlotImage = WeaponSlotImage_2; break;
	case 103: SlotImage = WeaponSlotImage_3; break;
	default: return;
	}

	if (SlotImage)
	{
		SlotImage->SetBrushFromTexture(Icon);
		SlotImage->SetVisibility(ESlateVisibility::Visible);
	}
}

void UPlayerHUDUI::UpdateInventorySlotSprite(int32 SlotIndex, UPaperSprite* Icon)
{
	UImage* SlotImage = nullptr;

	switch (SlotIndex)
	{
	case 0: SlotImage = InventorySlotImage_0; break;
	case 1: SlotImage = InventorySlotImage_1; break;
	case 2: SlotImage = InventorySlotImage_2; break;
	case 3: SlotImage = InventorySlotImage_3; break;
	case 4: SlotImage = InventorySlotImage_4; break;
	case 5: SlotImage = InventorySlotImage_5; break;
	case 6: SlotImage = InventorySlotImage_6; break;
	case 7: SlotImage = InventorySlotImage_7; break;
	case 8: SlotImage = InventorySlotImage_8; break;
	case 9: SlotImage = InventorySlotImage_9; break;
	case 100: SlotImage = WeaponSlotImage_0; break;
	case 101: SlotImage = WeaponSlotImage_1; break;
	case 102: SlotImage = WeaponSlotImage_2; break;
	case 103: SlotImage = WeaponSlotImage_3; break;
	default: return;
	}

	SetImageFromSprite(SlotImage, Icon);
}

void UPlayerHUDUI::HighlightSlot(int32 SlotIndex)
{
	UBorder* BGWidget = nullptr;

	switch (SlotIndex)
	{
	case 0:  BGWidget = InventorySlotBG_0; break;
	case 1:  BGWidget = InventorySlotBG_1; break;
	case 2:  BGWidget = InventorySlotBG_2; break;
	case 3:  BGWidget = InventorySlotBG_3; break;
	case 4:  BGWidget = InventorySlotBG_4; break;
	case 5:  BGWidget = InventorySlotBG_5; break;
	case 6:  BGWidget = InventorySlotBG_6; break;
	case 7:  BGWidget = InventorySlotBG_7; break;
	case 8:  BGWidget = InventorySlotBG_8; break;
	case 9:  BGWidget = InventorySlotBG_9; break;
	case 100: BGWidget = WeaponSlotBG_0; break;
	case 101: BGWidget = WeaponSlotBG_1; break;
	case 102: BGWidget = WeaponSlotBG_2; break;
	case 103: BGWidget = WeaponSlotBG_3; break;
	default: return;
	}

	if (BGWidget && HighlightedSlotBGSprite)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(HighlightedSlotBGSprite);
		Brush.ImageSize = FVector2D(64.f, 64.f); // Adjust to match your texture size
		BGWidget->SetBrush(Brush);
	}

	CurrentSwapSlot = SlotIndex;
}

void UPlayerHUDUI::ClearAllHighlights()
{
	TArray<UBorder*> AllBGs = {
		InventorySlotBG_0, InventorySlotBG_1, InventorySlotBG_2, InventorySlotBG_3, InventorySlotBG_4,
		InventorySlotBG_5, InventorySlotBG_6, InventorySlotBG_7, InventorySlotBG_8, InventorySlotBG_9,
		WeaponSlotBG_0, WeaponSlotBG_1, WeaponSlotBG_2, WeaponSlotBG_3
	};

	for (UBorder* BG : AllBGs)
	{
		if (BG && DefaultSlotBGSprite)
		{
			FSlateBrush Brush;
			Brush.SetResourceObject(DefaultSlotBGSprite);
			Brush.ImageSize = FVector2D(64.f, 64.f); // Adjust to match your texture size
			BG->SetBrush(Brush);
		}
	}

	CurrentSwapSlot = -1;
}

void UPlayerHUDUI::UpdateRunes(int32 NewRunes)
{
	if (CurrentRunes)
	{
		CurrentRunes->SetText(FText::AsNumber(NewRunes));
	}
}

void UPlayerHUDUI::SetInventoryPanelVisible(bool bVisible)
{
	if (InventoryPannel)
	{
		InventoryPannel->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UPlayerHUDUI::SetSlotToDefaultColor(int32 SlotIndex)
{
	UBorder* BGWidget = nullptr;

	switch (SlotIndex)
	{
	case 0:  BGWidget = InventorySlotBG_0; break;
	case 1:  BGWidget = InventorySlotBG_1; break;
	case 2:  BGWidget = InventorySlotBG_2; break;
	case 3:  BGWidget = InventorySlotBG_3; break;
	case 4:  BGWidget = InventorySlotBG_4; break;
	case 5:  BGWidget = InventorySlotBG_5; break;
	case 6:  BGWidget = InventorySlotBG_6; break;
	case 7:  BGWidget = InventorySlotBG_7; break;
	case 8:  BGWidget = InventorySlotBG_8; break;
	case 9:  BGWidget = InventorySlotBG_9; break;
	case 100: BGWidget = WeaponSlotBG_0; break;
	case 101: BGWidget = WeaponSlotBG_1; break;
	case 102: BGWidget = WeaponSlotBG_2; break;
	case 103: BGWidget = WeaponSlotBG_3; break;
	default: return;
	}

	if (BGWidget && DefaultSlotBGSprite)
	{
		FSlateBrush NewBrush;
		NewBrush.SetResourceObject(DefaultSlotBGSprite);
		NewBrush.ImageSize = FVector2D(64.f, 64.f); // Set to match your actual sprite size
		NewBrush.DrawAs = ESlateBrushDrawType::Image;

		BGWidget->SetBrush(NewBrush);
	}
}

void UPlayerHUDUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (DragIconWidget && DragIconWidget->IsVisible())
	{
		APlayerController* PC = GetOwningPlayer();
		if (!PC) return;

		FVector2D MousePosition;
		if (UWidgetLayoutLibrary::GetMousePositionScaledByDPI(PC, MousePosition.X, MousePosition.Y))
		{
			DragIconWidget->SetPositionInViewport(MousePosition, false);
		}
	}

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		int32 ViewX = 0, ViewY = 0;
		PC->GetViewportSize(ViewX, ViewY);

		if (CachedViewportSize != FIntPoint(ViewX, ViewY))
		{
			UpdateSafeArea();
		}
	}
}

void UPlayerHUDUI::ShowDragIcon(UTexture2D* Icon)
{
	if (DragIconWidget)
	{
		DragIconWidget->SetIcon(Icon);
	}
}

void UPlayerHUDUI::ShowDragIconSprite(UPaperSprite* Icon)
{
	if (DragIconWidget)
	{
		DragIconWidget->SetIconSprite(Icon);
	}
}

void UPlayerHUDUI::HideDragIcon()
{
	if (DragIconWidget)
	{
		DragIconWidget->SetIcon(nullptr); // Hides it
	}
}


FReply UPlayerHUDUI::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const FVector2D MousePosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());

	int32 NewHoveredIndex = -1;

	// Find which slot the mouse is over
	for (int32 i = 0; i < AllSlotWidgets.Num(); i++)
	{
		if (UWidget* Slott = AllSlotWidgets[i])
		{
			FGeometry SlotGeo = Slott->GetCachedGeometry();
			if (SlotGeo.IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
			{
				// Remap index: weapon slots (0�3) -> 100�103, inventory slots (4�13) -> 0�9
				NewHoveredIndex = (i <= 3) ? (100 + i) : (i - 4);
				break;
			}
		}
	}

	// If new hovered slot differs from the old
	if (HoveredSlotIndex != NewHoveredIndex)
	{
		if (HoveredSlotIndex != -1)
		{
			SetSlotToDefaultColor(HoveredSlotIndex);
		}

		if (NewHoveredIndex != -1)
		{
			HighlightSlot(NewHoveredIndex);
		}

		HoveredSlotIndex = NewHoveredIndex;
	}

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}



