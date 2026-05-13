// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UI/InteractWidget.h"
#include "Character/ALSPlayerController.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "PaperSprite.h"

void UInteractWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshButtonImage();
}

void UInteractWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const AALSPlayerController* ALSController = Cast<AALSPlayerController>(GetOwningPlayer());
	if (!ALSController)
	{
		ALSController = Cast<AALSPlayerController>(GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr);
	}

	if (ALSController && ALSController->CurrentInteractionInputType != LastInputType)
	{
		LastInputType = ALSController->CurrentInteractionInputType;
		RefreshButtonImage();
	}
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
	SetButtonImages(NewTexture, NewTexture, NewTexture);
}

void UInteractWidget::SetButtonImages(UTexture2D* KeyboardMouseTexture, UTexture2D* XboxTexture, UTexture2D* PlayStationTexture)
{
	KeyboardMouseButtonTexture = KeyboardMouseTexture;
	XboxButtonTexture = XboxTexture;
	PlayStationButtonTexture = PlayStationTexture;
	KeyboardMouseButtonSprite = nullptr;
	XboxButtonSprite = nullptr;
	PlayStationButtonSprite = nullptr;
	RefreshButtonImage();
}

void UInteractWidget::SetButtonSprite(UPaperSprite* NewSprite)
{
	SetButtonSprites(NewSprite, NewSprite, NewSprite);
}

void UInteractWidget::SetButtonSprites(UPaperSprite* KeyboardMouseSprite, UPaperSprite* XboxSprite, UPaperSprite* PlayStationSprite)
{
	KeyboardMouseButtonSprite = KeyboardMouseSprite;
	XboxButtonSprite = XboxSprite;
	PlayStationButtonSprite = PlayStationSprite;
	KeyboardMouseButtonTexture = nullptr;
	XboxButtonTexture = nullptr;
	PlayStationButtonTexture = nullptr;
	RefreshButtonImage();
}

void UInteractWidget::RefreshButtonImage()
{
	if (!ButtonImage)
	{
		return;
	}

	UPaperSprite* SpriteToUse = KeyboardMouseButtonSprite;
	UTexture2D* TextureToUse = KeyboardMouseButtonTexture;

	switch (LastInputType)
	{
	case EInteractionInputType::XboxGamepad:
		SpriteToUse = XboxButtonSprite ? XboxButtonSprite : KeyboardMouseButtonSprite;
		TextureToUse = XboxButtonTexture ? XboxButtonTexture : KeyboardMouseButtonTexture;
		break;
	case EInteractionInputType::PlayStationGamepad:
		SpriteToUse = PlayStationButtonSprite ? PlayStationButtonSprite : KeyboardMouseButtonSprite;
		TextureToUse = PlayStationButtonTexture ? PlayStationButtonTexture : KeyboardMouseButtonTexture;
		break;
	case EInteractionInputType::KeyboardMouse:
	default:
		break;
	}

	if (SpriteToUse)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(SpriteToUse);
		Brush.ImageSize = FVector2D(64.f, 64.f);
		ButtonImage->SetBrush(Brush);
	}
	else if (TextureToUse)
	{
		ButtonImage->SetBrushFromTexture(TextureToUse, true);
	}
}
