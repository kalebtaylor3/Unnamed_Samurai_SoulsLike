// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UI/LockOnWidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

ULockOnWidgetComponent::ULockOnWidgetComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetWidgetSpace(EWidgetSpace::Screen); // Use Screen so it always faces camera
	SetDrawAtDesiredSize(true);
	SetVisibility(false);
}

void ULockOnWidgetComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULockOnWidgetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!TargetPlayer) return;

	// Optionally update visibility here (e.g., when lock-on is active)
}
