// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "BonfireMenuWidget.generated.h"

class UBorder;
class UImage;
class UPanelWidget;

UCLASS()
class ALSV4_CPP_API UBonfireMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Index-based navigation
	void Navigate(int32 Direction);
	void SelectOption();
	void HighlightOption(int32 Index);

	void NavigateSkills(int32 Direction);
	void HighlightSkill(int32 Index);
	void UpdateSkillLevel(int32 Direction);
	void SelectLevelUp();
	void ConfirmLevelUp();
	void ExitLevelUpMode();

	bool isLevelMenuOpen = false;
	bool bCanConfirmLevelUp = true;

	UFUNCTION(BlueprintCallable)
	void RefreshLevelUpUI();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetBonfireLocationName(FText NewLocationName);


protected:
	UPROPERTY(meta = (BindWidget))
	UBorder* OptionBorder0;

	UPROPERTY(meta = (BindWidget))
	UBorder* OptionBorder1;

	UPROPERTY(meta = (BindWidget))
	UPanelWidget* BonfirePanel;

	UPROPERTY(meta = (BindWidget))
	UPanelWidget* LevelUpPanel;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* LocationNameTextBlock;

	// Add more if needed...

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	FLinearColor DefaultColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	FLinearColor HighlightColor = FLinearColor::Yellow;

	int32 CurrentIndex = 0;
	int32 MaxOptions = 2; // Set to how many you actually have

	void ClearOption(int32 Index);
	void ExecuteOption(int32 Index);

	UPROPERTY(meta = (BindWidget)) UBorder* SkillBorder0;
	UPROPERTY(meta = (BindWidget)) UBorder* SkillBorder1;
	UPROPERTY(meta = (BindWidget)) UBorder* SkillBorder2;

	UPROPERTY(meta = (BindWidget)) UImage* Left_0;
	UPROPERTY(meta = (BindWidget)) UImage* Right_0;
	UPROPERTY(meta = (BindWidget)) UImage* Left_1;
	UPROPERTY(meta = (BindWidget)) UImage* Right_1;
	UPROPERTY(meta = (BindWidget)) UImage* Left_2;
	UPROPERTY(meta = (BindWidget)) UImage* Right_2;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* UpdatedLevel_0;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* UpdatedLevel_1;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* UpdatedLevel_2;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentLevel;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NewLevel;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentLevel_0;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentLevel_1;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentLevel_2;

	UPROPERTY(meta = (BindWidget))
	UBorder* ConfirmBorder;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* RunesHeld;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RunesNeeded;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	FSlateColor NormalTextColor;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	FSlateColor NotEnoughRunesColor;

	int32 SkillIndex = 0;
	int32 MaxSkills = 4;

	void EnterLevelUpMode();
	//void ExitLevelUpMode();

	void ClearSkill(int32 Index);

private:
	UBorder* GetBorderByIndex(int32 Index);
	UBorder* GetSkillBorderByIndex(int32 Index);
	void SetArrowVisibility(int32 Index, bool bVisible);

	bool bCanSelectOption = true;

	void EnableOptionSelection();
	void EnableLevelUpConfirmation();

	int32 AllocatedVigor = 0;
	int32 AllocatedMind = 0;
	int32 AllocatedEndurance = 0;
};