// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/LootDropActor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Character/ALSBaseCharacter.h"
#include "BonfireSaveGame.h"
#include "Character/UI/InteractWidget.h"

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
				InteractUI->SetButtonImage(ActionIcon); // Your preloaded UTexture2D*
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
	if (!Player || !WeaponClass) return;

	if (UInventoryComponent* Inventory = Player->FindComponentByClass<UInventoryComponent>())
	{
		Inventory->BackpackWeapons.Add(WeaponClass);
		Inventory->UpdateInventoryUI();
		Player->ActiveLootDrop = nullptr;
		Inventory->SaveInventory();

		// Get the default object for the weapon to access icon and name
		if (UWeaponBase* WeaponCDO = WeaponClass->GetDefaultObject<UWeaponBase>())
		{
			if (PickupNotificationWidgetClass)
			{
				UItemPickupNotificationWidget* NotificationWidget = CreateWidget<UItemPickupNotificationWidget>(
					GetWorld(), PickupNotificationWidgetClass);

				if (NotificationWidget)
				{
					NotificationWidget->SetupPickupInfo(FText::FromName(WeaponCDO->WeaponName), WeaponCDO->WeaponIcon);
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
