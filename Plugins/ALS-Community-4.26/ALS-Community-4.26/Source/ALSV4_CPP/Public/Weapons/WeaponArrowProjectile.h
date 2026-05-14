// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponArrowProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class ALSV4_CPP_API AWeaponArrowProjectile : public AActor
{
	GENERATED_BODY()

public:
	AWeaponArrowProjectile();

	void InitializeArrow(float InDamage, float InSpeed, AActor* InIgnoredActor);

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ArrowMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow|Sticking")
	FName EnemyBodyTag = TEXT("EnemyBody");

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnArrowOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnArrowHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

private:
	void AddIgnoredActor(AActor* ActorToIgnore);
	bool IsIgnoredActor(const AActor* Actor) const;
	bool CanStickToHit(const FHitResult& Hit) const;
	bool TryFindEnemyBodyStickHit(AActor* HitActor, const FHitResult& OriginalHit, FHitResult& OutStickHit) const;
	void HandleEnemyHit(AActor* HitActor, const FHitResult& Hit);
	void StickArrowToHit(const FHitResult& Hit);

	UPROPERTY()
	AActor* IgnoredActor = nullptr;

	UPROPERTY()
	TSet<AActor*> IgnoredActors;

	UPROPERTY()
	TSet<AActor*> DamagedActors;

	float DamageAmount = 35.0f;
	FVector PreviousLocation = FVector::ZeroVector;
	bool bStuck = false;
};
