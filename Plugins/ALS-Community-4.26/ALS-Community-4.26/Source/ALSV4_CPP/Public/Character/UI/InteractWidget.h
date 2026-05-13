// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Character/UI/InteractionInputTypes.h"
#include "InteractWidget.generated.h"


class UTextBlock;
class UImage;
class UPaperSprite;

UCLASS()
class ALSV4_CPP_API UInteractWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Sets the action text, e.g., "Press to Rest" */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetActionText(const FText& NewText);

	/** Sets the button image dynamically */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetButtonImage(UTexture2D* NewTexture);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetButtonImages(UTexture2D* KeyboardMouseTexture, UTexture2D* XboxTexture, UTexture2D* PlayStationTexture);

	/** Sets the button image from a Paper2D sprite */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetButtonSprite(UPaperSprite* NewSprite);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetButtonSprites(UPaperSprite* KeyboardMouseSprite, UPaperSprite* XboxSprite, UPaperSprite* PlayStationSprite);

protected:
	// Called when widget is constructed
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Text displaying the action (e.g., "Press to Rest") */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionText;

	/** Image showing the button to press */
	UPROPERTY(meta = (BindWidget))
	UImage* ButtonImage;

private:
	UPROPERTY()
	UTexture2D* KeyboardMouseButtonTexture = nullptr;

	UPROPERTY()
	UTexture2D* XboxButtonTexture = nullptr;

	UPROPERTY()
	UTexture2D* PlayStationButtonTexture = nullptr;

	UPROPERTY()
	UPaperSprite* KeyboardMouseButtonSprite = nullptr;

	UPROPERTY()
	UPaperSprite* XboxButtonSprite = nullptr;

	UPROPERTY()
	UPaperSprite* PlayStationButtonSprite = nullptr;

	EInteractionInputType LastInputType = EInteractionInputType::KeyboardMouse;

	void RefreshButtonImage();
};
