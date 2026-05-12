// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "ItemPickupNotificationWidget.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;
class UPaperSprite;

UCLASS()
class ALSV4_CPP_API UItemPickupNotificationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetupPickupInfo(FText ItemName, UTexture2D* ItemIcon);

	UFUNCTION(BlueprintCallable)
	void SetupPickupInfoSprite(FText ItemName, UPaperSprite* ItemIcon);

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* FadeOut;

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* Image_WeaponIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_WeaponName;
};
