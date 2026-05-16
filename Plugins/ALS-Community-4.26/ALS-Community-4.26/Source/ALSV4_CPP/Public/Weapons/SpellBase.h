#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PaperSprite.h"
#include "Weapons/MagicTypes.h"
#include "SpellBase.generated.h"

class AALSBaseCharacter;
class AActor;
class UMagicWeaponBase;

UCLASS(Blueprintable, BlueprintType)
class ALSV4_CPP_API USpellBase : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	FName SpellName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	EMagicType MagicType = EMagicType::Magic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	UTexture2D* SpellIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	UPaperSprite* SpellIconSprite = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost")
	float FPCost = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost")
	float StaminaCost = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* CastMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cast", meta = (ClampMin = "0.0"))
	float CastReleaseDelay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cast")
	TSubclassOf<AActor> SpellActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cast")
	FName CastSocketName = TEXT("MagicSocket");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cast")
	FVector CastSpawnOffset = FVector(60.0f, 0.0f, 40.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cast")
	float AimTraceRange = 10000.0f;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spell")
	bool CastSpell(AALSBaseCharacter* Caster, UMagicWeaponBase* CastingWeapon);
	virtual bool CastSpell_Implementation(AALSBaseCharacter* Caster, UMagicWeaponBase* CastingWeapon);
};
