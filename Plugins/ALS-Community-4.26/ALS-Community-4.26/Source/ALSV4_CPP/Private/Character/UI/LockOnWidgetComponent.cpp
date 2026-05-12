// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UI/LockOnWidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

ULockOnWidgetComponent::ULockOnWidgetComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetWidgetSpace(EWidgetSpace::Screen); // Use Screen so it always faces camera
	SetDrawAtDesiredSize(false);
	SetDrawSize(WidgetDrawSize);
	SetVisibility(false);
}

void ULockOnWidgetComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (GEngine && GetWidgetClass())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
			FString::Printf(TEXT("LockOnWidget [%s] - Widget assigned, Size: %.0fx%.0f"),
				*GetOwner()->GetName(),
				WidgetDrawSize.X, WidgetDrawSize.Y));
	}
	else if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
			FString::Printf(TEXT("LockOnWidget [%s] - NO WIDGET CLASS ASSIGNED!"),
				*GetOwner()->GetName()));
	}
}

void ULockOnWidgetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!TargetPlayer) return;

	// Optionally update visibility here (e.g., when lock-on is active)
}
