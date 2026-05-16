// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/LootDropActor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Character/ALSBaseCharacter.h"
#include "BonfireSaveGame.h"
#include "Character/UI/InteractWidget.h"
#include "Kismet/GameplayStatics.h"

ALootDropActor::ALootDropActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Mesh
	LootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LootMesh"));
	SetRootComponent(LootMesh);
	LootMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Niagara FX
	LootFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LootFX"));
	LootFX->SetupAttachment(RootComponent);

	// Trigger Collider
	TriggerCollider = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerCollider"));
	TriggerCollider->SetupAttachment(RootComponent);
	TriggerCollider->SetSphereRadius(120.f);
	TriggerCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerCollider->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerCollider->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerCollider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Widget
	InteractWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractWidget"));
	InteractWidgetComponent->SetupAttachment(RootComponent);
	InteractWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	InteractWidgetComponent->SetDrawSize(FVector2D(250.f, 80.f));
	InteractWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	InteractWidgetComponent->SetVisibility(false); // Start hidden
}

void ALootDropActor::BeginPlay()
{
	Super::BeginPlay();

	if (InteractWidgetClass)
	{
		InteractWidgetComponent->SetWidgetClass(InteractWidgetClass.Get());
	}

	if (UGameplayStatics::DoesSaveGameExist(TEXT("BonfireSlot"), 0))
	{
		if (UBonfireSaveGame* SaveData = Cast<UBonfireSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("BonfireSlot"), 0)))
		{
			if (SaveData->CollectedLootIDs.Contains(LootID))
			{
				Destroy(); // Already picked up in a previous session
				return;
			}
		}
	}

	TriggerCollider->OnComponentBeginOverlap.AddDynamic(this, &ALootDropActor::OnOverlapBegin);
	TriggerCollider->OnComponentEndOverlap.AddDynamic(this, &ALootDropActor::OnOverlapEnd);
}

void ALootDropActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (AALSBaseCharacter* Player = Cast<AALSBaseCharacter>(OtherActor))
	{
		Player->ActiveLootDrop = this;
		InteractWidgetComponent->SetVisibility(true);

		if (UUserWidget* RawWidget = InteractWidgetComponent->GetUserWidgetObject())
		{
			if (UInteractWidget* InteractUI = Cast<UInteractWidget>(RawWidget))
			{
				InteractUI->SetActionText(ActionText);
				if (KeyboardMouseActionSprite || XboxActionSprite || PlayStationActionSprite || ActionSprite)
				{
					InteractUI->SetButtonSprites(
						KeyboardMouseActionSprite ? KeyboardMouseActionSprite : ActionSprite,
						XboxActionSprite ? XboxActionSprite : ActionSprite,
						PlayStationActionSprite ? PlayStationActionSprite : ActionSprite);
				}
				else
				{
					InteractUI->SetButtonImages(
						KeyboardMouseActionIcon ? KeyboardMouseActionIcon : ActionIcon,
						XboxActionIcon ? XboxActionIcon : ActionIcon,
						PlayStationActionIcon ? PlayStationActionIcon : ActionIcon);
				}
			}
		}
	}
}

void ALootDropActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AALSBaseCharacter* Player = Cast<AALSBaseCharacter>(OtherActor))
	{
		Player->ActiveLootDrop = nullptr;
		InteractWidgetComponent->SetVisibility(false);
	}
}

void ALootDropActor::GiveLootToPlayer(AALSBaseCharacter* Player)
{
	if (!Player || (!WeaponClass && !SpellClass)) return;

	if (UInventoryComponent* Inventory = Player->FindComponentByClass<UInventoryComponent>())
	{
		FText PickupName = FText::FromString(TEXT("Item"));
		UTexture2D* PickupIcon = nullptr;
		UPaperSprite* PickupIconSprite = nullptr;

		if (WeaponClass)
		{
			Inventory->BackpackWeapons.Add(WeaponClass);

			if (UWeaponBase* WeaponCDO = WeaponClass->GetDefaultObject<UWeaponBase>())
			{
				PickupName = FText::FromName(WeaponCDO->WeaponName);
				PickupIcon = WeaponCDO->WeaponIcon;
				PickupIconSprite = WeaponCDO->WeaponIconSprite;
			}
		}
		else if (SpellClass)
		{
			const int32 MaxEquippedSpells = 4;
			int32 EquippedSlot = INDEX_NONE;

			for (int32 Index = 0; Index < MaxEquippedSpells; ++Index)
			{
				if (!Inventory->EquippedSpells.IsValidIndex(Index))
				{
					Inventory->EquippedSpells.SetNum(Index + 1);
				}

				if (!Inventory->EquippedSpells[Index])
				{
					EquippedSlot = Index;
					break;
				}
			}

			if (EquippedSlot != INDEX_NONE)
			{
				Inventory->EquippedSpells[EquippedSlot] = SpellClass;
				if (!Inventory->CurrentSpell)
				{
					Inventory->EquipSpellByIndex(EquippedSlot);
				}
			}
			else
			{
				Inventory->BackpackSpells.Add(SpellClass);
			}

			if (USpellBase* SpellCDO = SpellClass->GetDefaultObject<USpellBase>())
			{
				PickupName = FText::FromName(SpellCDO->SpellName);
				PickupIcon = SpellCDO->SpellIcon;
				PickupIconSprite = SpellCDO->SpellIconSprite;
			}
		}

		Inventory->UpdateInventoryUI();
		Player->ActiveLootDrop = nullptr;
		Inventory->SaveInventory();

		if (PickupNotificationWidgetClass)
		{
			UItemPickupNotificationWidget* NotificationWidget = CreateWidget<UItemPickupNotificationWidget>(
				GetWorld(), PickupNotificationWidgetClass);

			if (NotificationWidget)
			{
				if (PickupIconSprite)
				{
					NotificationWidget->SetupPickupInfoSprite(PickupName, PickupIconSprite);
				}
				else
				{
					NotificationWidget->SetupPickupInfo(PickupName, PickupIcon);
				}
				NotificationWidget->AddToViewport();

				// Play fade out animation
				NotificationWidget->PlayAnimation(NotificationWidget->FadeOut);

				// Remove widget AFTER fade finishes (match duration to animation length)
				FTimerHandle RemoveHandle;
				FTimerDelegate RemoveDelegate = FTimerDelegate::CreateLambda([NotificationWidget]()
					{
						if (NotificationWidget)
						{
							NotificationWidget->RemoveFromParent();
						}
					});
				GetWorld()->GetTimerManager().SetTimer(RemoveHandle, RemoveDelegate, 5.0f /* animation length */, false);
			}
		}

		// Optionally play pickup sound or FX here

		Destroy(); // Remove the loot drop from the world

		if (UGameplayStatics::DoesSaveGameExist(TEXT("BonfireSlot"), 0))
		{
			if (UBonfireSaveGame* SaveData = Cast<UBonfireSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("BonfireSlot"), 0)))
			{
				if (!SaveData->CollectedLootIDs.Contains(LootID))
				{
					SaveData->CollectedLootIDs.Add(LootID);
					UGameplayStatics::SaveGameToSlot(SaveData, TEXT("BonfireSlot"), 0);
				}
			}
		}
	}
}
