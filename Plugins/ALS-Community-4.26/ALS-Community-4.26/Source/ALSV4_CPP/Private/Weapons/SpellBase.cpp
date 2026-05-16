#include "Weapons/SpellBase.h"

#include "Weapons/MagicWeaponBase.h"

bool USpellBase::CastSpell_Implementation(AALSBaseCharacter* Caster, UMagicWeaponBase* CastingWeapon)
{
	if (!CastingWeapon)
	{
		return false;
	}

	return CastingWeapon->SpawnMagicActor(Caster, SpellActorClass, CastSocketName, CastSpawnOffset, AimTraceRange);
}
