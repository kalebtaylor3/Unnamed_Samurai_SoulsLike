// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "LockOnWidgetComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALSV4_CPP_API ULockOnWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	ULockOnWidgetComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	APlayerController* TargetPlayer = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	FVector2D WidgetDrawSize = FVector2D(100.0f, 100.0f);
};
