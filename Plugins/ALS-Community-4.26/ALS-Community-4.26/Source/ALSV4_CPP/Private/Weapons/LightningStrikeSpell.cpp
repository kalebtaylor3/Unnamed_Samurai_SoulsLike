#include "Weapons/LightningStrikeSpell.h"

#include "AI/EnemyHealthComponent.h"
#include "Character/ALSBaseCharacter.h"
#include "Character/ALSPlayerCameraManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Weapons/LightningStrikeSpellActor.h"
#include "Weapons/MagicWeaponBase.h"

ULightningStrikeSpell::ULightningStrikeSpell()
{
	SpellName = TEXT("Lightning Strike");
	MagicType = EMagicType::Lightning;
	FPCost = 45.0f;
	StaminaCost = 25.0f;
	CastReleaseDelay = 0.55f;
	CastSpawnOffset = FVector::ZeroVector;
	SpellActorClass = ALightningStrikeSpellActor::StaticClass();
}

bool ULightningStrikeSpell::CastSpell_Implementation(AALSBaseCharacter* Caster, UMagicWeaponBase* CastingWeapon)
{
	if (!Caster || !Caster->GetWorld())
	{
		return false;
	}

	TSubclassOf<AActor> ActorClass = SpellActorClass;
	if (!ActorClass)
	{
		ActorClass = ALightningStrikeSpellActor::StaticClass();
	}
	if (!ActorClass)
	{
		return false;
	}

	const FVector Forward = GetCasterForward(Caster);
	AActor* LockedTarget = GetLockedTarget(Caster);

	FVector DesiredCenter = Caster->GetActorLocation() + Forward * SpawnDistanceInFront;
	if (LockedTarget)
	{
		const FVector TargetAimLocation = GetTargetAimLocation(LockedTarget);
		if (FVector::Dist(Caster->GetActorLocation(), TargetAimLocation) <= LockedTargetCastRange)
		{
			DesiredCenter = TargetAimLocation;
		}
		else
		{
			LockedTarget = nullptr;
		}
	}

	const FVector SpawnLocation = FindGroundLocation(Caster, DesiredCenter);
	const FRotator SpawnRotation = Forward.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Caster;
	SpawnParams.Instigator = Caster;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* SpawnedActor = Caster->GetWorld()->SpawnActor<AActor>(ActorClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (ALightningStrikeSpellActor* LightningActor = Cast<ALightningStrikeSpellActor>(SpawnedActor))
	{
		LightningActor->InitializeLightningStrike(Caster, LockedTarget, SpawnLocation);
	}

	return SpawnedActor != nullptr;
}

FVector ULightningStrikeSpell::GetCasterForward(AALSBaseCharacter* Caster) const
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

AActor* ULightningStrikeSpell::GetLockedTarget(AALSBaseCharacter* Caster) const
{
	if (!Caster)
	{
		return nullptr;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(Caster->GetController());
	if (!PlayerController)
	{
		return nullptr;
	}

	const AALSPlayerCameraManager* CameraManager = Cast<AALSPlayerCameraManager>(PlayerController->PlayerCameraManager);
	if (!CameraManager || !CameraManager->bIsTargetLocked || !CameraManager->LockedTarget)
	{
		return nullptr;
	}

	const UEnemyHealthComponent* HealthComponent = CameraManager->LockedTarget->FindComponentByClass<UEnemyHealthComponent>();
	return HealthComponent && !HealthComponent->IsDeadOrOutOfHealth() ? CameraManager->LockedTarget : nullptr;
}

FVector ULightningStrikeSpell::GetTargetAimLocation(AActor* TargetActor) const
{
	if (!TargetActor)
	{
		return FVector::ZeroVector;
	}

	if (const USkeletalMeshComponent* MeshComponent = TargetActor->FindComponentByClass<USkeletalMeshComponent>())
	{
		static const FName TargetSocketName(TEXT("TargetLockSocket"));
		if (MeshComponent->DoesSocketExist(TargetSocketName))
		{
			return MeshComponent->GetSocketLocation(TargetSocketName);
		}
	}

	FVector Origin = FVector::ZeroVector;
	FVector Extent = FVector::ZeroVector;
	TargetActor->GetActorBounds(false, Origin, Extent);
	return !Origin.IsNearlyZero() ? Origin : TargetActor->GetActorLocation();
}

FVector ULightningStrikeSpell::FindGroundLocation(AALSBaseCharacter* Caster, const FVector& DesiredLocation) const
{
	if (!Caster || !Caster->GetWorld())
	{
		return DesiredLocation;
	}

	const FVector TraceStart = DesiredLocation + FVector(0.0f, 0.0f, GroundTraceHeight);
	const FVector TraceEnd = DesiredLocation - FVector(0.0f, 0.0f, GroundTraceDepth);

	FHitResult Hit;
	FCollisionQueryParams QueryParams(FName(TEXT("LightningStrikeGroundTrace")), false, Caster);
	QueryParams.AddIgnoredActor(Caster);

	const bool bHitGround = Caster->GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
	return bHitGround ? FVector(Hit.ImpactPoint) + FVector(0.0f, 0.0f, GroundOffset) : DesiredLocation;
}
