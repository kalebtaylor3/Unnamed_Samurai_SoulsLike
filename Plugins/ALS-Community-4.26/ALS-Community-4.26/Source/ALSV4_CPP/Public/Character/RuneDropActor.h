// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RuneDropActor.generated.h"

UCLASS()
class ALSV4_CPP_API ARuneDropActor : public AActor
{
	GENERATED_BODY()

public:
	ARuneDropActor();

	UPROPERTY(EditAnywhere)
	int32 RuneAmount;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	class USphereComponent* Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rune Drop")
	class UNiagaraComponent* NiagaraEffect;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* Overlapped, AActor* Other,
		UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
