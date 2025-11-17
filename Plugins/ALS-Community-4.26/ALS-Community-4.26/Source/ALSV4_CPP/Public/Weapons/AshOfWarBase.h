// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AshOfWarBase.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, EditInlineNew)
class ALSV4_CPP_API UAshOfWarBase : public UObject
{
	GENERATED_BODY()

public:
	/** Optional: name used in UI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AshOfWar")
	FText AshName;

	/** Called when player activates the Ash (e.g., via input) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AshOfWar")
	void ActivateAsh(AALSBaseCharacter* Character);

	virtual void ActivateAsh_Implementation(AALSBaseCharacter* Character);
	
};
