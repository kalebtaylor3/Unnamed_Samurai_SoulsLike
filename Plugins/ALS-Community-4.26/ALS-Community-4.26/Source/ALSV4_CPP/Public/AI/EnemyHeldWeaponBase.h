// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Particles/ParticleSystem.h"
#include "EnemyHeldWeaponBase.generated.h"

class UBoxComponent;

UCLASS()
class ALSV4_CPP_API AEnemyHeldWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AEnemyHeldWeaponBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* DamageHitbox;

	void EnableDamageCollision(float InDamageAmount);
	void DisableDamageCollision();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* HitEffect;


	UFUNCTION()
	void OnDamageHitboxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	float CurrentDamageAmount = 0.f;

	UPROPERTY()
	TSet<AActor*> AlreadyDamagedActors;
};
