#include "Weapons/IceWallSpell.h"

#include "Character/ALSBaseCharacter.h"
#include "GameFramework/Controller.h"
#include "Weapons/IceWallSpellActor.h"
#include "Weapons/MagicWeaponBase.h"

UIceWallSpell::UIceWallSpell()
{
	SpellName = TEXT("Ice Wall");
	MagicType = EMagicType::Magic;
	FPCost = 50.0f;
	StaminaCost = 30.0f;
	CastReleaseDelay = 0.72f;
	CastSpawnOffset = FVector::ZeroVector;
	SpellActorClass = AIceWallSpellActor::StaticClass();
}

bool UIceWallSpell::CastSpell_Implementation(AALSBaseCharacter* Caster, UMagicWeaponBase* CastingWeapon)
{
	if (!Caster || !Caster->GetWorld())
	{
		return false;
	}

	TSubclassOf<AActor> ActorClass = SpellActorClass;
	if (!ActorClass)
	{
		ActorClass = AIceWallSpellActor::StaticClass();
	}
	if (!ActorClass)
	{
		return false;
	}

	const FVector Forward = GetCasterForward(Caster);
	const FVector SpawnLocation = FindGroundSpawnLocation(Caster, Forward);
	const FRotator SpawnRotation = Forward.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Caster;
	SpawnParams.Instigator = Caster;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* SpawnedActor = Caster->GetWorld()->SpawnActor<AActor>(ActorClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (AIceWallSpellActor* IceWallActor = Cast<AIceWallSpellActor>(SpawnedActor))
	{
		IceWallActor->InitializeIceWall(Caster);
	}

	return SpawnedActor != nullptr;
}

FVector UIceWallSpell::GetCasterForward(AALSBaseCharacter* Caster) const
{
	if (!Caster)
	{
		return FVector::ForwardVector;
	}

	if (const AController* Controller = Caster->GetController())
	{
		FRotator ControlRotation = Controller->GetControlRotation();
		ControlRotation.Pitch = 0.0f;
		ControlRotation.Roll = 0.0f;
		return ControlRotation.Vector().GetSafeNormal();
	}

	FVector Forward = Caster->GetActorForwardVector();
	Forward.Z = 0.0f;
	return Forward.GetSafeNormal();
}

FVector UIceWallSpell::FindGroundSpawnLocation(AALSBaseCharacter* Caster, const FVector& Forward) const
{
	FVector SpawnOffset = Caster->GetActorTransform().TransformVector(CastSpawnOffset);
	SpawnOffset.Z = CastSpawnOffset.Z;
	const FVector DesiredLocation = Caster->GetActorLocation() + Forward * SpawnDistanceInFront + SpawnOffset;
	if (!Caster->GetWorld())
	{
		return DesiredLocation;
	}

	const FVector TraceStart = DesiredLocation + FVector(0.0f, 0.0f, GroundTraceHeight);
	const FVector TraceEnd = DesiredLocation - FVector(0.0f, 0.0f, GroundTraceDepth);

	FHitResult Hit;
	FCollisionQueryParams QueryParams(FName(TEXT("IceWallGroundTrace")), false, Caster);
	QueryParams.AddIgnoredActor(Caster);

	const bool bHitGround = Caster->GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
	return bHitGround ? FVector(Hit.ImpactPoint) + FVector(0.0f, 0.0f, GroundOffset) : DesiredLocation;
}
