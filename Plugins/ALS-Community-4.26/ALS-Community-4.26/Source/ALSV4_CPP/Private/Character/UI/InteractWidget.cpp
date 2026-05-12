// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UI/InteractWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "PaperSprite.h"

void UInteractWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// Optional: Set default text/image here if needed
}

void UInteractWidget::SetActionText(const FText& NewText)
{
	if (ActionText)
	{
		ActionText->SetText(NewText);
	}
}

void UInteractWidget::SetButtonImage(UTexture2D* NewTexture)
{
	if (ButtonImage && NewTexture)
	{
		ButtonImage->SetBrushFromTexture(NewTexture, true);
	}
}

void UInteractWidget::SetButtonSprite(UPaperSprite* NewSprite)
{
	if (ButtonImage && NewSprite)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(NewSprite);
		Brush.ImageSize = FVector2D(64.f, 64.f);
		ButtonImage->SetBrush(Brush);
	}
}
