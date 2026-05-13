// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Character/UI/BonfireMenuWidget.h"
#include "Bonfire.generated.h"


class UBoxComponent;
class UWidgetComponent;
class UPaperSprite;

UCLASS()
class ALSV4_CPP_API ABonfire : public AActor
{
	GENERATED_BODY()

public:
	ABonfire();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bonfire")
	FText LocationName = FText::FromString("Unknown Location");

	void SaveGameState(AALSBaseCharacter* Character);

	UPROPERTY()
	UBonfireMenuWidget* BonfireMenuWidget;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BonfireMesh;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* InteractionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* InteractionWidget;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UBonfireMenuWidget> BonfireMenuWidgetClass;

public:
	void OpenLevelUpUI(ACharacter* PlayerCharacter);

	UPROPERTY(BlueprintReadOnly)
	bool bCanInteract = false;

	UFUNCTION(BlueprintCallable)
	void Interact(AALSBaseCharacter* Character);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	UTexture2D* ButtonTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	UTexture2D* KeyboardMouseButtonTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	UTexture2D* XboxButtonTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	UTexture2D* PlayStationButtonTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	UPaperSprite* ButtonSprite;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	UPaperSprite* KeyboardMouseButtonSprite;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	UPaperSprite* XboxButtonSprite;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	UPaperSprite* PlayStationButtonSprite;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FText ActionText = FText::FromString("Rest at Bonfire");

	UFUNCTION(BlueprintCallable)
	void ExitBonfire(AALSBaseCharacter* Character, bool levelMenuOpen);


};
