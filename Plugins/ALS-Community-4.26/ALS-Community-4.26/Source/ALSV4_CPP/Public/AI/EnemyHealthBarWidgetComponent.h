// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "EnemyHealthBarWidgetComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALSV4_CPP_API UEnemyHealthBarWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UEnemyHealthBarWidgetComponent();
};