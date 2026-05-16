#include "Weapons/MagicWeaponBase.h"

#include "Character/ALSBaseCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Weapons/BaseCastBeam.h"
#include "Weapons/BaseCastProjectile.h"
#include "Weapons/HeldWeaponBase.h"
#include "Weapons/SpellBase.h"

UMagicWeaponBase::UMagicWeaponBase()
{
	OverlayType = EALSOverlayState::Staff;
}

bool UMagicWeaponBase::IsSpellCompatible(const USpellBase* Spell) const
{
	return Spell && Spell->MagicType == MagicType;
}

bool UMagicWeaponBase::BaseCast_Implementation(AALSBaseCharacter* Caster)
{
	return SpawnMagicActor(Caster, BaseCastActorClass, CastSocketName, CastSpawnOffset, MagicAimTraceRange);
}

bool UMagicWeaponBase::CastEquippedSpell_Implementation(AALSBaseCharacter* Caster, USpellBase* Spell)
{
	if (!IsSpellCompatible(Spell))
	{
		return false;
	}

	return Spell->CastSpell(Caster, this);
}

bool UMagicWeaponBase::SpawnMagicActor(AALSBaseCharacter* Caster, TSubclassOf<AActor> ActorClass, FName SocketName, FVector SpawnOffset, float TraceRange) const
{
	if (!Caster || !ActorClass || !Caster->GetWorld())
	{
		return false;
	}

	const FVector SpawnLocation = GetCastSpawnLocation(Caster, SocketName, SpawnOffset);
	const FRotator SpawnRotation = GetCastAimRotation(Caster, SpawnLocation, TraceRange);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Caster;
	SpawnParams.Instigator = Caster;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* SpawnedActor = Caster->GetWorld()->SpawnActor<AActor>(ActorClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (ABaseCastProjectile* MagicProjectile = Cast<ABaseCastProjectile>(SpawnedActor))
	{
		MagicProjectile->InitializeMagicProjectile(Caster);
	}
	else if (ABaseCastBeam* MagicBeam = Cast<ABaseCastBeam>(SpawnedActor))
	{
		MagicBeam->InitializeMagicBeam(Caster);
	}

	return SpawnedActor != nullptr;
}

FVector UMagicWeaponBase::GetCastSpawnLocation(AALSBaseCharacter* Caster, FName SocketName, FVector SpawnOffset) const
{
	if (!Caster)
	{
		return FVector::ZeroVector;
	}

	if (Caster->HeldWeaponActor && Caster->HeldWeaponActor->WeaponMesh)
	{
		if (Caster->HeldWeaponActor->WeaponMesh->DoesSocketExist(SocketName))
		{
			return Caster->HeldWeaponActor->WeaponMesh->GetSocketLocation(SocketName);
		}
	}

	return Caster->GetActorLocation() + Caster->GetActorTransform().TransformVector(SpawnOffset);
}

FRotator UMagicWeaponBase::GetCastAimRotation(AALSBaseCharacter* Caster, const FVector& SpawnLocation, float TraceRange) const
{
	if (!Caster)
	{
		return FRotator::ZeroRotator;
	}

	APlayerController* PlayerController = Cast<APlayerController>(Caster->GetController());
	if (!PlayerController)
	{
		return Caster->GetActorRotation();
	}

	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FVector TraceStart = ViewLocation;
	const FVector TraceEnd = TraceStart + ViewRotation.Vector() * TraceRange;

	FHitResult Hit;
	FCollisionQueryParams QueryParams(FName(TEXT("MagicAimTrace")), false, Caster);
	const bool bHit = Caster->GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	const FVector AimPoint = bHit ? Hit.ImpactPoint : TraceEnd;
	return (AimPoint - SpawnLocation).Rotation();
}
