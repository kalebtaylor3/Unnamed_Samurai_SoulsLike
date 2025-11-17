// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractWidget.generated.h"


class UTextBlock;
class UImage;

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

protected:
	// Called when widget is constructed
	virtual void NativeConstruct() override;

	/** Text displaying the action (e.g., "Press to Rest") */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionText;

	/** Image showing the button to press */
	UPROPERTY(meta = (BindWidget))
	UImage* ButtonImage;
};