// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EnemyHealthBarWidgetComponent.h"


UEnemyHealthBarWidgetComponent::UEnemyHealthBarWidgetComponent()
{
	// Display settings
	SetDrawSize(FVector2D(150.0f, 20.0f)); // Adjust as needed
	SetWidgetSpace(EWidgetSpace::Screen);
	SetPivot(FVector2D(0.5f, 1.0f)); // Bottom center of widget aligns to socket
	SetVisibility(false); // Start hidden
	SetTwoSided(true);

	// Optional: make sure it doesn’t block line traces
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}