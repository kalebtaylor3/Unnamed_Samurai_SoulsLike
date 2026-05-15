// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UI/BonfireMenuWidget.h"
#include "Character/ALSBaseCharacter.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"

void UBonfireMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateSafeArea();
	OpenMainMenu();
}

void UBonfireMenuWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	int32 ViewX = 0;
	int32 ViewY = 0;
	PC->GetViewportSize(ViewX, ViewY);
	if (ViewX > 0 && ViewY > 0 && CachedViewportSize != FIntPoint(ViewX, ViewY))
	{
		UpdateSafeArea();
	}
}

void UBonfireMenuWidget::UpdateSafeArea()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	int32 ViewX = 0;
	int32 ViewY = 0;
	PC->GetViewportSize(ViewX, ViewY);
	if (ViewX <= 0 || ViewY <= 0)
	{
		return;
	}

	CachedViewportSize = FIntPoint(ViewX, ViewY);

	constexpr float DesiredAspect = 16.0f / 9.0f;
	const float ViewAspect = static_cast<float>(ViewX) / static_cast<float>(ViewY);

	float WorldWidthPx = 0.0f;
	float WorldHeightPx = 0.0f;
	float WorldXPx = 0.0f;
	float WorldYPx = 0.0f;

	if (ViewAspect > DesiredAspect)
	{
		WorldHeightPx = static_cast<float>(ViewY);
		WorldWidthPx = WorldHeightPx * DesiredAspect;
		WorldXPx = 0.5f * (static_cast<float>(ViewX) - WorldWidthPx);
	}
	else
	{
		WorldWidthPx = static_cast<float>(ViewX);
		WorldHeightPx = WorldWidthPx / DesiredAspect;
		WorldYPx = 0.5f * (static_cast<float>(ViewY) - WorldHeightPx);
	}

	float DPIScale = UWidgetLayoutLibrary::GetViewportScale(this);
	if (DPIScale <= 0.0f)
	{
		DPIScale = 1.0f;
	}

	const FVector2D SafePosition(WorldXPx / DPIScale, WorldYPx / DPIScale);
	const FVector2D SafeSize(WorldWidthPx / DPIScale, WorldHeightPx / DPIScale);

	auto ApplySafeAreaToWidget = [SafePosition, SafeSize](UWidget* Widget)
	{
		if (!Widget)
		{
			return;
		}

		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot))
		{
			CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
			CanvasSlot->SetAlignment(FVector2D(0.0f, 0.0f));
			CanvasSlot->SetPosition(SafePosition);
			CanvasSlot->SetSize(SafeSize);
		}

		Widget->SetRenderTransformPivot(FVector2D(0.0f, 0.0f));
		Widget->SetRenderScale(FVector2D(1.0f, 1.0f));
	};

	if (SafeArea_16_9)
	{
		ApplySafeAreaToWidget(SafeArea_16_9);
	}
	else
	{
		ApplySafeAreaToWidget(BonfirePanel);
		ApplySafeAreaToWidget(LevelUpPanel);
	}
}

void UBonfireMenuWidget::OpenMainMenu()
{
	SetVisibility(ESlateVisibility::Visible);

	if (UWidget* RootWidget = GetRootWidget())
	{
		RootWidget->SetVisibility(ESlateVisibility::Visible);
	}

	if (BonfirePanel)
	{
		BonfirePanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (LevelUpPanel)
	{
		LevelUpPanel->SetVisibility(ESlateVisibility::Hidden);
	}

	isLevelMenuOpen = false;
	CurrentIndex = 0;
	ClearOption(1);
	HighlightOption(CurrentIndex);
}

void UBonfireMenuWidget::Navigate(int32 Direction)
{
	if (isLevelMenuOpen)
		return;

	int32 PreviousIndex = CurrentIndex;

	if (Direction == 0 && CurrentIndex > 0) // Up
	{
		CurrentIndex--;
	}
	else if (Direction == 1 && CurrentIndex < MaxOptions - 1) // Down
	{
		CurrentIndex++;
	}

	if (PreviousIndex != CurrentIndex)
	{
		ClearOption(PreviousIndex);
		HighlightOption(CurrentIndex);
	}
}

void UBonfireMenuWidget::SelectOption()
{
	ExecuteOption(CurrentIndex);
}

void UBonfireMenuWidget::SelectLevelUp()
{
	if (!bCanConfirmLevelUp)
		return;

	bCanConfirmLevelUp = false;
	ConfirmLevelUp();

	// Re-enable confirmation after a short delay
}

void UBonfireMenuWidget::EnableLevelUpConfirmation()
{
	bCanConfirmLevelUp = true;
}

void UBonfireMenuWidget::HighlightOption(int32 Index)
{
	if (UBorder* Border = GetBorderByIndex(Index))
	{
		Border->SetBrushColor(HighlightColor);
	}
}

void UBonfireMenuWidget::ClearOption(int32 Index)
{
	if (UBorder* Border = GetBorderByIndex(Index))
	{
		Border->SetBrushColor(DefaultColor);
	}
}

void UBonfireMenuWidget::ExecuteOption(int32 Index)
{
	if (!bCanSelectOption) return; // block input during cooldown
	bCanSelectOption = false;

	// Reactivate input after short delay
	FTimerHandle SelectionCooldownTimer;
	GetWorld()->GetTimerManager().SetTimer(SelectionCooldownTimer, this, &UBonfireMenuWidget::EnableOptionSelection, 0.1f, false);

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	AALSBaseCharacter* ALSChar = PC ? Cast<AALSBaseCharacter>(PC->GetCharacter()) : nullptr;

	switch (Index)
	{
	case 0:
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, TEXT("Resting..."));
		ALSChar->Rest();
		break;

	case 1:
		EnterLevelUpMode();
		break;

	default:
		break;
	}
}

void UBonfireMenuWidget::SetBonfireLocationName(FText NewLocationName)
{
	if (LocationNameTextBlock) // Assume you've created and exposed a UTextBlock* reference in the .h
	{
		LocationNameTextBlock->SetText(NewLocationName);
	}
}

void UBonfireMenuWidget::EnterLevelUpMode()
{
	// Clear main menu highlight
	ClearOption(CurrentIndex);

	// Clear all skill-related highlights
	for (int32 i = 0; i <= MaxSkills - 1; i++)
	{
		ClearSkill(i);
	}
	ClearSkill(3); // Confirm border highlight if previously selected

	if (BonfirePanel) BonfirePanel->SetVisibility(ESlateVisibility::Hidden);
	if (LevelUpPanel) LevelUpPanel->SetVisibility(ESlateVisibility::Visible);

	SkillIndex = 0;
	HighlightSkill(SkillIndex);
	RefreshLevelUpUI();

	isLevelMenuOpen = true;
}

void UBonfireMenuWidget::ExitLevelUpMode()
{
	if (BonfirePanel) BonfirePanel->SetVisibility(ESlateVisibility::Visible);
	if (LevelUpPanel) LevelUpPanel->SetVisibility(ESlateVisibility::Hidden);

	SkillIndex = 0;

	isLevelMenuOpen = false;
}


void UBonfireMenuWidget::NavigateSkills(int32 Direction)
{
	int32 Prev = SkillIndex;

	if (Direction == 0 && SkillIndex > 0)
		SkillIndex--;
	else if (Direction == 1 && SkillIndex < MaxSkills - 1)
		SkillIndex++;

	if (SkillIndex != Prev)
	{
		ClearSkill(Prev);
		HighlightSkill(SkillIndex);
	}
}

void UBonfireMenuWidget::HighlightSkill(int32 Index)
{
	if (UBorder* Border = GetSkillBorderByIndex(Index))
		Border->SetBrushColor(HighlightColor);

	SetArrowVisibility(Index, true);
}

void UBonfireMenuWidget::UpdateSkillLevel(int32 Direction)
{
	switch (SkillIndex)
	{
	case 0:
		AllocatedVigor = FMath::Clamp(AllocatedVigor + Direction, 0, 99);
		break;

	case 1:
		AllocatedMind = FMath::Clamp(AllocatedMind + Direction, 0, 99);
		break;

	case 2:
		AllocatedEndurance = FMath::Clamp(AllocatedEndurance + Direction, 0, 99);
		break;
	}

	RefreshLevelUpUI(); // Live update everything
}

void UBonfireMenuWidget::ClearSkill(int32 Index)
{
	if (UBorder* Border = GetSkillBorderByIndex(Index))
		Border->SetBrushColor(DefaultColor);

	SetArrowVisibility(Index, false);
}

UBorder* UBonfireMenuWidget::GetSkillBorderByIndex(int32 Index)
{
	switch (Index)
	{
	case 0: return SkillBorder0;
	case 1: return SkillBorder1;
	case 2: return SkillBorder2;
	case 3: return ConfirmBorder; // Confirm button
	default: return nullptr;
	}
}

void UBonfireMenuWidget::ConfirmLevelUp()
{
	if (!bCanConfirmLevelUp)
		return;

	if (SkillIndex != 3)
		return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	AALSBaseCharacter* ALSChar = PC ? Cast<AALSBaseCharacter>(PC->GetCharacter()) : nullptr;

	if (ALSChar && ALSChar->PlayerStats)
	{
		ALSChar->PlayerStats->IncreaseLevel(AllocatedVigor, AllocatedMind, AllocatedEndurance);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("Stats confirmed and leveled up!"));
		}
	}

	// Reset skill allocations
	AllocatedVigor = 0;
	AllocatedMind = 0;
	AllocatedEndurance = 0;
	RefreshLevelUpUI();
	bCanConfirmLevelUp = false;
	FTimerHandle ConfirmCooldownTimer;
	GetWorld()->GetTimerManager().SetTimer(ConfirmCooldownTimer, this, &UBonfireMenuWidget::EnableLevelUpConfirmation, 0.2f, false);
}

void UBonfireMenuWidget::RefreshLevelUpUI()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	AALSBaseCharacter* ALSChar = PC ? Cast<AALSBaseCharacter>(PC->GetCharacter()) : nullptr;

	if (ALSChar && ALSChar->PlayerStats)
	{
		int32 Vigor = ALSChar->PlayerStats->VigorLevel;
		int32 Mind = ALSChar->PlayerStats->MindLevel;
		int32 Endurance = ALSChar->PlayerStats->EnduranceLevel;
		int32 CurrentLVL = ALSChar->PlayerStats->CurrentLevel;

		if (CurrentLevel_0)
			CurrentLevel_0->SetText(FText::AsNumber(Vigor));
		if (CurrentLevel_1)
			CurrentLevel_1->SetText(FText::AsNumber(Mind));
		if (CurrentLevel_2)
			CurrentLevel_2->SetText(FText::AsNumber(Endurance));

		if (UpdatedLevel_0)
			UpdatedLevel_0->SetText(FText::AsNumber(Vigor + AllocatedVigor));
		if (UpdatedLevel_1)
			UpdatedLevel_1->SetText(FText::AsNumber(Mind + AllocatedMind));
		if (UpdatedLevel_2)
			UpdatedLevel_2->SetText(FText::AsNumber(Endurance + AllocatedEndurance));

		if (CurrentLevel)
			CurrentLevel->SetText(FText::AsNumber(CurrentLVL));

		const int32 AllocatedTotal = AllocatedVigor + AllocatedMind + AllocatedEndurance;
		if (NewLevel)
			NewLevel->SetText(FText::AsNumber(CurrentLVL + AllocatedTotal));

		// --- Rune Logic ---
		int32 RequiredRunes = 0;
		for (int32 i = 1; i <= AllocatedTotal; ++i)
		{
			RequiredRunes += ALSChar->PlayerStats->GetRunesRequiredForLevel(CurrentLVL + i - 1);
		}

		const int32 CurrentRunes = ALSChar->PlayerStats->CurrentRunes;

		if (RunesHeld)
			RunesHeld->SetText(FText::AsNumber(CurrentRunes));

		if (RunesNeeded)
		{
			RunesNeeded->SetText(FText::AsNumber(RequiredRunes));
			FSlateColor ColorToUse = (CurrentRunes >= RequiredRunes) ? NormalTextColor : NotEnoughRunesColor;
			RunesNeeded->SetColorAndOpacity(ColorToUse);
		}
	}
}


void UBonfireMenuWidget::SetArrowVisibility(int32 Index, bool bVisible)
{
	ESlateVisibility isVisible = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden;

	switch (Index)
	{
	case 0:
		if (Left_0) Left_0->SetVisibility(isVisible);
		if (Right_0) Right_0->SetVisibility(isVisible);
		break;
	case 1:
		if (Left_1) Left_1->SetVisibility(isVisible);
		if (Right_1) Right_1->SetVisibility(isVisible);
		break;
	case 2:
		if (Left_2) Left_2->SetVisibility(isVisible);
		if (Right_2) Right_2->SetVisibility(isVisible);
		break;
	}
}

void UBonfireMenuWidget::EnableOptionSelection()
{
	bCanSelectOption = true;
}

UBorder* UBonfireMenuWidget::GetBorderByIndex(int32 Index)
{
	switch (Index)
	{
	case 0: return OptionBorder0;
	case 1: return OptionBorder1;
		// Add more if needed
	default: return nullptr;
	}
}
