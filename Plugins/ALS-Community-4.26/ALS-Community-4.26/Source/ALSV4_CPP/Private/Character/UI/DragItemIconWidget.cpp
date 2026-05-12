// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UI/DragItemIconWidget.h"
#include "PaperSprite.h"

void UDragItemIconWidget::SetIcon(UTexture2D* NewIcon)
{
	if (IconImage)
	{
		if (NewIcon)
		{
			IconImage->SetBrushFromTexture(NewIcon);
			SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UDragItemIconWidget::SetIconSprite(UPaperSprite* NewIcon)
{
	if (IconImage)
	{
		if (NewIcon)
		{
			FSlateBrush Brush;
			Brush.SetResourceObject(NewIcon);
			Brush.ImageSize = FVector2D(64.f, 64.f);
			IconImage->SetBrush(Brush);
			SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
