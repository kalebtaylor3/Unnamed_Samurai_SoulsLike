// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UI/DragItemIconWidget.h"

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