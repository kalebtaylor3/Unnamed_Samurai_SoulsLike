#pragma once

#include "CoreMinimal.h"
#include "Weapons/SpellBase.h"
#include "LightningStrikeSpell.generated.h"

UCLASS(Blueprintable, BlueprintType)
class ALSV4_CPP_API ULightningStrikeSpell : public USpellBase
{
	GENERATED_BODY()

public:
	ULightningStrikeSpell();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lightning Strike|Cast")
	float SpawnDistanceInFront = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lightning Strike|Cast")
	float LockedTargetCastRange = 3600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lightning Strike|Cast")
	float GroundTraceHeight = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lightning Strike|Cast")
	float GroundTraceDepth = 2200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lightning Strike|Cast")
	float GroundOffset = 0.0f;

	virtual bool CastSpell_Implementation(AALSBaseCharacter* Caster, UMagicWeaponBase* CastingWeapon) override;

private:
	FVector GetCasterForward(AALSBaseCharacter* Caster) const;
	AActor* GetLockedTarget(AALSBaseCharacter* Caster) const;
	FVector GetTargetAimLocation(AActor* TargetActor) const;
	FVector FindGroundLocation(AALSBaseCharacter* Caster, const FVector& DesiredLocation) const;
};
