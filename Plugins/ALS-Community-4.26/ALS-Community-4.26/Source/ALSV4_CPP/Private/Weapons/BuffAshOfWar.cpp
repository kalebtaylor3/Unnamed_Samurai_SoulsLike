// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/BuffAshOfWar.h"
#include "Character/ALSBaseCharacter.h"
#include "Weapons/HeldWeaponBase.h"

void UBuffAshOfWar::ActivateAsh_Implementation(AALSBaseCharacter* Character)
{
	if (!Character || !BuffMontage) return;

	Character->PlayAnimMontage(BuffMontage);
}

void UBuffAshOfWar::OnBuffNotify(AALSBaseCharacter* Character)
{
	if (!Character || !BuffEffect) return;


	//the time in the animation to actually apply the buff affects to the weapon...

	//AHeldWeaponBase* HeldWeapon = Character->GetHeldWeapon(); // You'll need to expose this

	//if (HeldWeapon)
	//{
	//	UNiagaraComponent* FX = UNiagaraFunctionLibrary::SpawnSystemAttached(
	//		BuffEffect,
	//		HeldWeapon->GetMesh(), // Make sure HeldWeapon exposes its mesh
	//		NAME_None,
	//		FVector::ZeroVector,
	//		FRotator::ZeroRotator,
	//		EAttachLocation::SnapToTarget,
	//		true
	//	);

	//	// Optional: store reference to FX if you want to stop or fade it later

	//	// Optional: set a timer to remove or fade the FX after BuffDuration
	//}

}