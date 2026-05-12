// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UI/ItemPickupNotificationWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "PaperSprite.h"

void UItemPickupNotificationWidget::SetupPickupInfo(FText ItemName, UTexture2D* ItemIcon)
{
	if (Text_WeaponName)
	{
		Text_WeaponName->SetText(ItemName);
	}

	if (Image_WeaponIcon && ItemIcon)
	{
		Image_WeaponIcon->SetBrushFromTexture(ItemIcon);
	}
}

void UItemPickupNotificationWidget::SetupPickupInfoSprite(FText ItemName, UPaperSprite* ItemIcon)
{
	if (Text_WeaponName)
	{
		Text_WeaponName->SetText(ItemName);
	}

	if (Image_WeaponIcon && ItemIcon)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(ItemIcon);
		Brush.ImageSize = FVector2D(64.f, 64.f);
		Image_WeaponIcon->SetBrush(Brush);
	}
}
