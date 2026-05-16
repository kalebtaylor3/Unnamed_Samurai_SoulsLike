#pragma once

#include "CoreMinimal.h"
#include "Weapons/SpellBase.h"
#include "IceWallSpell.generated.h"

UCLASS(Blueprintable, BlueprintType)
class ALSV4_CPP_API UIceWallSpell : public USpellBase
{
	GENERATED_BODY()

public:
	UIceWallSpell();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ice Wall|Cast")
	float SpawnDistanceInFront = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ice Wall|Cast")
	float GroundTraceHeight = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ice Wall|Cast")
	float GroundTraceDepth = 850.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ice Wall|Cast")
	float GroundOffset = 0.0f;

	virtual bool CastSpell_Implementation(AALSBaseCharacter* Caster, UMagicWeaponBase* CastingWeapon) override;

private:
	FVector GetCasterForward(AALSBaseCharacter* Caster) const;
	FVector FindGroundSpawnLocation(AALSBaseCharacter* Caster, const FVector& Forward) const;
};
