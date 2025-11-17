// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "HeldWeaponBase.generated.h"

UCLASS()
class ALSV4_CPP_API AHeldWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AHeldWeaponBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* DamageHitbox;

	void EnableDamageCollision(float InDamageAmount);
	void DisableDamageCollision();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* HitEffect;

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	float CurrentDamageAmount = 0.f;

	UPROPERTY()
	TSet<AActor*> AlreadyDamagedActors;

	UFUNCTION()
	void OnDamageHitboxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

};