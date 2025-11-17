// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include <Components/Image.h>
#include "DragItemIconWidget.generated.h"

/**
 * 
 */

UCLASS()
class ALSV4_CPP_API UDragItemIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetIcon(UTexture2D* NewIcon);
	

	UPROPERTY(meta = (BindWidget))
	UImage* IconImage;

};
