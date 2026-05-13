// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapons/WeaponBase.h"
#include "Character/UI/ItemPickupNotificationWidget.h"
#include "LootDropActor.generated.h"

class UStaticMeshComponent;
class UNiagaraComponent;
class USphereComponent;
class UWidgetComponent;
class UInteractWidget;
class UTexture2D;
class UPaperSprite;

UCLASS()
class ALSV4_CPP_API ALootDropActor : public AActor
{
	GENERATED_BODY()

public:
	ALootDropActor();

protected:
	virtual void BeginPlay() override;

	/** Called when player overlaps */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	/** Called when player leaves trigger */
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Static mesh for visual representation */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* LootMesh;

	/** Niagara FX (glow, particles, etc.) */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UNiagaraComponent* LootFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TSubclassOf<UWeaponBase> WeaponClass;

	/** Collider for detecting overlap */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* TriggerCollider;

	/** Tooltip widget */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UWidgetComponent* InteractWidgetComponent;

	/** Cached widget reference */
	UPROPERTY()
	UInteractWidget* InteractWidgetInstance;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	TSubclassOf<UInteractWidget> InteractWidgetClass;

	/** Action text for interaction (e.g., "Pick up Rune") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText ActionText = FText::FromString("Pick up item");

	/** Icon to display on the button (e.g., gamepad icon) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	UTexture2D* ActionIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	UTexture2D* KeyboardMouseActionIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	UTexture2D* XboxActionIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	UTexture2D* PlayStationActionIcon;

	/** Sprite to display on the button when using a sprite sheet */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	UPaperSprite* ActionSprite;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	UPaperSprite* KeyboardMouseActionSprite;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	UPaperSprite* XboxActionSprite;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	UPaperSprite* PlayStationActionSprite;

	UFUNCTION(BlueprintCallable)
	void GiveLootToPlayer(class AALSBaseCharacter* Player);

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UItemPickupNotificationWidget> PickupNotificationWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save")
	FName LootID;
};
