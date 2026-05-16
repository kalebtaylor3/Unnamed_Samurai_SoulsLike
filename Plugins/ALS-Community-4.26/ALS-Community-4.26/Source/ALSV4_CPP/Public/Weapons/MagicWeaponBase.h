#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponBase.h"
#include "Weapons/MagicTypes.h"
#include "MagicWeaponBase.generated.h"

class AALSBaseCharacter;
class AActor;
class USpellBase;

UCLASS(Blueprintable, BlueprintType)
class ALSV4_CPP_API UMagicWeaponBase : public UWeaponBase
{
	GENERATED_BODY()

public:
	UMagicWeaponBase();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic")
	EMagicType MagicType = EMagicType::Magic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Base Cast")
	float BaseCastFPCost = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Base Cast")
	float BaseCastStaminaCost = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Base Cast")
	UAnimMontage* BaseCastMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Base Cast")
	TSubclassOf<AActor> BaseCastActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Cast")
	FName CastSocketName = TEXT("MagicSocket");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Cast")
	FVector CastSpawnOffset = FVector(60.0f, 0.0f, 40.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magic|Cast")
	float MagicAimTraceRange = 10000.0f;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Magic")
	bool IsSpellCompatible(const USpellBase* Spell) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Magic")
	bool BaseCast(AALSBaseCharacter* Caster);
	virtual bool BaseCast_Implementation(AALSBaseCharacter* Caster);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Magic")
	bool CastEquippedSpell(AALSBaseCharacter* Caster, USpellBase* Spell);
	virtual bool CastEquippedSpell_Implementation(AALSBaseCharacter* Caster, USpellBase* Spell);

	UFUNCTION(BlueprintCallable, Category = "Magic")
	bool SpawnMagicActor(AALSBaseCharacter* Caster, TSubclassOf<AActor> ActorClass, FName SocketName, FVector SpawnOffset, float TraceRange) const;

private:
	FVector GetCastSpawnLocation(AALSBaseCharacter* Caster, FName SocketName, FVector SpawnOffset) const;
	FRotator GetCastAimRotation(AALSBaseCharacter* Caster, const FVector& SpawnLocation, float TraceRange) const;
};
