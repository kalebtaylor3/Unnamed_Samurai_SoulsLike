// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/InventoryComponent.h"
#include "BonfireSaveGame.h"
#include <Character/ALSBaseCharacter.h>

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	RefreshEquippedWeaponsFromBackpack();
	FTimerHandle DelayHandle;
	GetWorld()->GetTimerManager().SetTimer(DelayHandle, this, &UInventoryComponent::UpdateInventoryUI, 0.1f, false);
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UInventoryComponent::EquipWeaponByIndex(int32 Index)
{
	EquippedIndex = Index;

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, FString::Printf(TEXT("Equipped Index: %d"), Index));

	// Equip weapon normally
	if (!EquippedWeapons.IsValidIndex(Index) || !EquippedWeapons[Index])
	{
		CurrentWeapon = nullptr;

		if (auto* OwnerChar = Cast<AALSBaseCharacter>(GetOwner()))
		{
			if (auto* CombatComp = OwnerChar->FindComponentByClass<UCombatComponent>())
			{
				CombatComp->CurrentWeapon = nullptr;
				OwnerChar->PlayerHUDWidget->UpdateWeaponIcon(nullptr);
				OwnerChar->PlayerHUDWidget->UpdateAshOfWarIcon(nullptr);
				OwnerChar->SetOverlayState(EALSOverlayState::Default);
				SaveInventory();
			}
		}
		return;
	}

	if (auto* OwnerChar = Cast<AALSBaseCharacter>(GetOwner()))
	{
		OwnerChar->SetOverlayState(EALSOverlayState::Default);
	}

	UWeaponBase* NewWeapon = NewObject<UWeaponBase>(this, EquippedWeapons[Index]);
	if (NewWeapon)
	{
		CurrentWeapon = NewWeapon;

		if (auto* OwnerChar = Cast<AALSBaseCharacter>(GetOwner()))
		{
			if (auto* CombatComp = OwnerChar->FindComponentByClass<UCombatComponent>())
			{
				CombatComp->CurrentWeapon = NewWeapon;
				OwnerChar->PlayerHUDWidget->UpdateWeaponIcon(NewWeapon->WeaponIcon);
				OwnerChar->PlayerHUDWidget->UpdateAshOfWarIcon(NewWeapon->AshOfWarIcon);
				OwnerChar->SetOverlayState(NewWeapon->OverlayType);
				SaveInventory();
			}
		}
	}
}

void UInventoryComponent::CycleNextWeapon()
{
	if (!bCanCycleWeapon) return; // Block if cooldown is active

	bCanCycleWeapon = false;

	const int32 TotalSlots = EquippedWeapons.Num() + 1; // +1 for "no weapon"
	EquippedIndex = (EquippedIndex + 1) % TotalSlots;

	if (auto* OwnerChar = Cast<AALSBaseCharacter>(GetOwner()))
	{
		OwnerChar->SetOverlayState(EALSOverlayState::Default);

		FTimerHandle EquipDelayHandle;
		GetWorld()->GetTimerManager().SetTimer(EquipDelayHandle, [this]()
			{
				EquipWeaponByIndex(EquippedIndex);
				bCanCycleWeapon = true; // Re-enable cycling after equip
			}, 0.20f, false); // You can adjust this time (0.15s�0.25s feels good)
	}
	else
	{
		EquipWeaponByIndex(EquippedIndex);
		bCanCycleWeapon = true;
	}
}


void UInventoryComponent::RefreshEquippedWeaponsFromBackpack()
{
	EquippedWeapons.Empty();

	const int32 MaxEquipped = 4;
	const int32 NumToEquip = FMath::Min(BackpackWeapons.Num(), MaxEquipped);

	// Track which indices to remove
	TSet<int32> EquippedIndices;

	for (int32 i = 0; i < BackpackWeapons.Num(); ++i)
	{
		if (EquippedWeapons.Num() >= MaxEquipped)
			break;

		if (BackpackWeapons[i])
		{
			EquippedWeapons.Add(BackpackWeapons[i]);
			EquippedIndices.Add(i);
		}
	}

	// Remove by index from end to start
	EquippedIndices.Sort([](int32 A, int32 B) { return B < A; });
	for (int32 Index : EquippedIndices)
	{
		BackpackWeapons.RemoveAt(Index);
	}
}


void UInventoryComponent::UseActivePotion()
{
	if (auto* OwnerChar = Cast<AALSBaseCharacter>(GetOwner()))
	{
		if (OwnerChar->CombatSystem->bIsAttacking)
			return;

		UAnimInstance* AnimInstance = OwnerChar->GetMesh()->GetAnimInstance();
		switch (ActivePotion)
		{
		case EPotionType::HP:
			if (CurrentHPPotions > 0)
			{
				--CurrentHPPotions;
				bHealing = true;
				if (AnimInstance)
				{
					AnimInstance->Montage_Play(HPHealMontage);//AnimInstance->Montage_Play(OneHJumpAttackLightAttackMontage);
				}
				// Heal player here
				OwnerChar->PlayerStats->RestoreHealth(400.f);
			}
			break;
		case EPotionType::FP:
			if (CurrentFPPotions > 0)
			{
				--CurrentFPPotions;
				bHealing = true;
				if (AnimInstance)
				{
					AnimInstance->Montage_Play(FPHealMontage);//AnimInstance->Montage_Play(OneHJumpAttackLightAttackMontage);
				}
				// Restore FP here
				OwnerChar->PlayerStats->RestoreFP(60.f);
			}
			break;
		}

		UpdatePotionHUD();
	}
}

void UInventoryComponent::TogglePotionType()
{
	ActivePotion = (ActivePotion == EPotionType::HP) ? EPotionType::FP : EPotionType::HP;
	UpdatePotionHUD();
}

void UInventoryComponent::UpdatePotionHUD()
{
	if (auto* OwnerChar = Cast<AALSBaseCharacter>(GetOwner()))
	{
		if (OwnerChar->PlayerHUDWidget)
		{
			if (ActivePotion == EPotionType::HP)
			{
				OwnerChar->PlayerHUDWidget->UpdatePotionIcon(HPPotionIcon);
			}
			else
			{
				OwnerChar->PlayerHUDWidget->UpdatePotionIcon(FPPotionIcon);
			}

			OwnerChar->PlayerHUDWidget->UpdatePotionCount(
				ActivePotion == EPotionType::HP ? CurrentHPPotions : CurrentFPPotions);
		}
	}
}

void UInventoryComponent::UpdateInventoryUI()
{
	if (auto* OwnerChar = Cast<AALSBaseCharacter>(GetOwner()))
	{
		if (!OwnerChar->PlayerHUDWidget) return;

		// ----- Equipped Weapons -----
		for (int32 i = 0; i < 4; ++i)
		{
			UTexture2D* EquippedIcon = EmptySlotIcon;

			if (EquippedWeapons.IsValidIndex(i) && EquippedWeapons[i])
			{
				if (UWeaponBase* DefaultWeapon = EquippedWeapons[i]->GetDefaultObject<UWeaponBase>())
				{
					EquippedIcon = DefaultWeapon->WeaponIcon;
				}
			}

			OwnerChar->PlayerHUDWidget->UpdateInventorySlot(100 + i, EquippedIcon);
		}

		// ----- Backpack -----
		for (int32 i = 0; i < 10; ++i)
		{
			UTexture2D* IconToUse = EmptySlotIcon;

			if (BackpackWeapons.IsValidIndex(i) && BackpackWeapons[i])
			{
				if (UWeaponBase* DefaultWeapon = BackpackWeapons[i]->GetDefaultObject<UWeaponBase>())
				{
					IconToUse = DefaultWeapon->WeaponIcon;
				}
			}

			OwnerChar->PlayerHUDWidget->UpdateInventorySlot(i, IconToUse);
		}
	}
}

void UInventoryComponent::SelectSlot(int32 Index)
{
	AALSBaseCharacter* OwnerChar = Cast<AALSBaseCharacter>(GetOwner());
	if (!OwnerChar || !bIsInventoryOpen || bSwapCooldownActive)
		return;

	// First click (select)
	if (!bIsSwapping)
	{
		SelectedSlotIndex = Index;
		bIsSwapping = true;
		OwnerChar->PlayerHUDWidget->HighlightSlot(Index);

		UTexture2D* Icon = GetIconAtIndex(Index);
		OwnerChar->PlayerHUDWidget->ShowDragIcon(Icon);
		OwnerChar->PlayerHUDWidget->UpdateInventorySlot(Index, EmptySlotIcon);

		return;
	}

	// Ignore selecting same slot
	if (Index == SelectedSlotIndex)
		return;

	// === Swap ===
	SwapWeapons(SelectedSlotIndex, Index);
	UpdateInventoryUI();

	bSwapCooldownActive = true;
	GetWorld()->GetTimerManager().SetTimer(SwapCooldownTimer, [this]()
		{
			bSwapCooldownActive = false;
		}, 0.1f, false);

	ClearSelection();
}


void UInventoryComponent::ClearSelection()
{
	AALSBaseCharacter* OwnerChar = Cast<AALSBaseCharacter>(GetOwner());
	if (!OwnerChar)
		return;

	bIsSwapping = false;
	SelectedSlotIndex = -1;

	OwnerChar->PlayerHUDWidget->ClearAllHighlights();
	OwnerChar->PlayerHUDWidget->HideDragIcon();
}

UTexture2D* UInventoryComponent::GetIconAtIndex(int32 Index) const
{
	if (Index >= 0 && Index < BackpackWeapons.Num() && BackpackWeapons[Index])
	{
		if (UWeaponBase* Weapon = BackpackWeapons[Index]->GetDefaultObject<UWeaponBase>())
		{
			return Weapon->WeaponIcon;
		}
	}
	else if (Index >= 100 && Index < 104)
	{
		int32 EquippedSlot = Index - 100;
		if (EquippedWeapons.IsValidIndex(EquippedSlot) && EquippedWeapons[EquippedSlot])
		{
			if (UWeaponBase* Weapon = EquippedWeapons[EquippedSlot]->GetDefaultObject<UWeaponBase>())
			{
				return Weapon->WeaponIcon;
			}
		}
	}

	return nullptr;
}


void UInventoryComponent::SwapWeapons(int32 SlotA, int32 SlotB)
{
	// Slot type flags
	bool AEquipped = false;
	bool BEquipped = false;

	// Real indices inside their respective arrays
	int32 AReal = 0;
	int32 BReal = 0;

	// Lambda to interpret slot index and return reference to item (expand arrays if needed)
	auto InterpretSlot = [](int32 Index, TArray<TSubclassOf<UWeaponBase>>& Equipped, TArray<TSubclassOf<UWeaponBase>>& Backpack, int32& OutRealIndex, bool& bOutIsEquipped) -> TSubclassOf<UWeaponBase>*
		{
			if (Index >= 100 && Index <= 103)
			{
				bOutIsEquipped = true;
				OutRealIndex = Index - 100;

				if (!Equipped.IsValidIndex(OutRealIndex))
				{
					Equipped.SetNum(OutRealIndex + 1);
				}
				return &Equipped[OutRealIndex];
			}
			else if (Index >= 0 && Index <= 9)
			{
				bOutIsEquipped = false;
				OutRealIndex = Index;

				if (!Backpack.IsValidIndex(OutRealIndex))
				{
					Backpack.SetNum(OutRealIndex + 1);
				}
				return &Backpack[OutRealIndex];
			}

			return nullptr;
		};

	// Get references to both slots (including empty ones)
	TSubclassOf<UWeaponBase>* SlotARef = InterpretSlot(SlotA, EquippedWeapons, BackpackWeapons, AReal, AEquipped);
	TSubclassOf<UWeaponBase>* SlotBRef = InterpretSlot(SlotB, EquippedWeapons, BackpackWeapons, BReal, BEquipped);

	if (!SlotARef || !SlotBRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("SwapWeapons failed: Invalid slot(s) A:%d B:%d"), SlotA, SlotB);
		return;
	}

	// Swap contents
	TSubclassOf<UWeaponBase> Temp = *SlotARef;
	*SlotARef = *SlotBRef;
	*SlotBRef = Temp;

	// Re-equip if the equipped slot was affected
	if (auto* OwnerChar = Cast<AALSBaseCharacter>(GetOwner()))
	{
		FTimerHandle EquipDelayHandle;
		const float DelayDuration = 0.10f;

		if (AEquipped && AReal == EquippedIndex)
		{
			OwnerChar->SetOverlayState(EALSOverlayState::Default); // Reset overlay state first
			GetWorld()->GetTimerManager().SetTimer(EquipDelayHandle, [this, AReal]()
				{
					EquipWeaponByIndex(AReal);
				}, DelayDuration, false);
		}
		else if (BEquipped && BReal == EquippedIndex)
		{
			OwnerChar->SetOverlayState(EALSOverlayState::Default); // Reset overlay state first
			GetWorld()->GetTimerManager().SetTimer(EquipDelayHandle, [this, BReal]()
				{
					EquipWeaponByIndex(BReal);
				}, DelayDuration, false);
		}
	}

	SaveInventory();
}

void UInventoryComponent::SaveInventory()
{
	UBonfireSaveGame* SaveData = nullptr;

	// ?? Try to load existing save so we don't wipe other data
	if (UGameplayStatics::DoesSaveGameExist(TEXT("BonfireSlot"), 0))
	{
		SaveData = Cast<UBonfireSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("BonfireSlot"), 0));
	}

	// If no save exists, create a new one
	if (!SaveData)
	{
		SaveData = Cast<UBonfireSaveGame>(UGameplayStatics::CreateSaveGameObject(UBonfireSaveGame::StaticClass()));
	}

	// ?? Just update inventory fields
	SaveData->SavedEquippedWeapons = EquippedWeapons;
	SaveData->SavedBackpackWeapons = BackpackWeapons;
	SaveData->SavedEquippedIndex = EquippedIndex;

	// ? Save back to the slot
	UGameplayStatics::SaveGameToSlot(SaveData, TEXT("BonfireSlot"), 0);
}



