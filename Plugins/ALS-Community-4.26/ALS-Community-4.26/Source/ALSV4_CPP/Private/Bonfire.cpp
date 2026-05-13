// Fill out your copyright notice in the Description page of Project Settings.


#include "Bonfire.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Character/PlayerStatsComponent.h"
#include "GameFramework/Character.h"
#include "Character/ALSCharacter.h"
#include "Character/UI/InteractWidget.h"
#include "Character/UI/BonfireMenuWidget.h"
#include "AI/ALSAIController.h"
#include "BonfireSaveGame.h"
#include "MyGameInstance.h"
#include "AI/EnemyCombatComponent.h"
#include "GameFramework/PlayerController.h"

ABonfire::ABonfire()
{
	PrimaryActorTick.bCanEverTick = false;

	BonfireMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BonfireMesh"));
	RootComponent = BonfireMesh;

	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	InteractionBox->SetupAttachment(RootComponent);
	InteractionBox->SetBoxExtent(FVector(100.f));
	InteractionBox->SetCollisionProfileName("Trigger");

	InteractionWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionWidget"));
	InteractionWidget->SetupAttachment(RootComponent);
	InteractionWidget->SetWidgetSpace(EWidgetSpace::Screen);
	InteractionWidget->SetDrawSize(FVector2D(300, 50));
	InteractionWidget->SetRelativeLocation(FVector(0, 0, 100));

	InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &ABonfire::OnOverlapBegin);
	InteractionBox->OnComponentEndOverlap.AddDynamic(this, &ABonfire::OnOverlapEnd);
}

void ABonfire::BeginPlay()
{
	Super::BeginPlay();
	InteractionWidget->SetVisibility(false);
}


void ABonfire::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (AALSBaseCharacter* ALSChar = Cast<AALSBaseCharacter>(OtherActor))
	{
		bCanInteract = true;
		ALSChar->bCanInteractWithBonfire = true;
		ALSChar->ActiveBonfire = this;
		InteractionWidget->SetVisibility(true);

		if (UUserWidget* RawWidget = InteractionWidget->GetUserWidgetObject())
		{
			if (UInteractWidget* InteractUI = Cast<UInteractWidget>(RawWidget))
			{
				InteractUI->SetActionText(ActionText);
				if (KeyboardMouseButtonSprite || XboxButtonSprite || PlayStationButtonSprite || ButtonSprite)
				{
					InteractUI->SetButtonSprites(
						KeyboardMouseButtonSprite ? KeyboardMouseButtonSprite : ButtonSprite,
						XboxButtonSprite ? XboxButtonSprite : ButtonSprite,
						PlayStationButtonSprite ? PlayStationButtonSprite : ButtonSprite);
				}
				else
				{
					InteractUI->SetButtonImages(
						KeyboardMouseButtonTexture ? KeyboardMouseButtonTexture : ButtonTexture,
						XboxButtonTexture ? XboxButtonTexture : ButtonTexture,
						PlayStationButtonTexture ? PlayStationButtonTexture : ButtonTexture);
				}
			}
		}

		// Show interaction widget
	}
}

void ABonfire::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

	if (AALSBaseCharacter* ALSChar = Cast<AALSBaseCharacter>(OtherActor))
	{
		bCanInteract = false;
		ALSChar->bCanInteractWithBonfire = false;
		ALSChar->ActiveBonfire = nullptr;
		InteractionWidget->SetVisibility(false);
		// Hide interaction widget
	}

	// You can hide/close the UI if needed
}

void ABonfire::OpenLevelUpUI(ACharacter* PlayerCharacter)
{
	if (!BonfireMenuWidgetClass) return;

	if (!BonfireMenuWidget)
	{
		BonfireMenuWidget = CreateWidget<UBonfireMenuWidget>(GetWorld(), BonfireMenuWidgetClass);
		if (BonfireMenuWidget)
		{
			BonfireMenuWidget->AddToViewport();
			BonfireMenuWidget->HighlightOption(0);
			BonfireMenuWidget->SetBonfireLocationName(LocationName);

			if (AALSBaseCharacter* ALSChar = Cast<AALSBaseCharacter>(PlayerCharacter))
			{
				ALSChar->CurrentBonfireMenu = BonfireMenuWidget;
			}

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, TEXT("Bonfire Menu Opened"));
			}
		}
	}
}

void ABonfire::ExitBonfire(AALSBaseCharacter* Character, bool levelMenuOpen)
{
	if (!Character) return;

	if (levelMenuOpen)
	{
		if (BonfireMenuWidget)
		{
			BonfireMenuWidget->ExitLevelUpMode();
			return;
		}
	}

	// Close the menu
	if (BonfireMenuWidget)
	{
		BonfireMenuWidget->isLevelMenuOpen = false;
		BonfireMenuWidget->RemoveFromParent();
		BonfireMenuWidget = nullptr;
	}

	SaveGameState(Character);

	// Stop the looped sitting animation
	Character->StopAnimMontage(Character->SitLoopMontage);

	// Play the stand-up montage
	UAnimMontage* StandMontage = Character->StandUpMontage;
	if (StandMontage)
	{
		float Duration = Character->PlayAnimMontage(StandMontage);

		// Delay cleanup until stand-up animation finishes
		FTimerHandle CleanupTimer;
		Character->GetWorldTimerManager().SetTimer(CleanupTimer, [Character]()
			{
				Character->bIsResting = false;
				Character->CurrentBonfireMenu = nullptr;
				Character->ActiveBonfire = nullptr;

				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, TEXT("Exited Bonfire (Post Stand-Up)"));
					Character->bIsInvisibleToEnemies = false;
				}

			}, Duration, false);
	}
	else
	{
		// Fallback cleanup if no montage
		Character->bIsResting = false;
		Character->CurrentBonfireMenu = nullptr;
		Character->ActiveBonfire = nullptr;
	}
}

void ABonfire::SaveGameState(AALSBaseCharacter* Character)
{
	UBonfireSaveGame* SaveData = nullptr;

	// ? Load existing save if available
	if (UGameplayStatics::DoesSaveGameExist(TEXT("BonfireSlot"), 0))
	{
		SaveData = Cast<UBonfireSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("BonfireSlot"), 0));
	}

	// ?? If not found, create a new one
	if (!SaveData)
	{
		SaveData = Cast<UBonfireSaveGame>(UGameplayStatics::CreateSaveGameObject(UBonfireSaveGame::StaticClass()));
	}

	// ?? Save player info
	SaveData->PlayerLocation = Character->GetActorLocation();
	SaveData->PlayerRotation = Character->GetActorRotation();
	SaveData->Health = Character->PlayerStats->CurrentHealth;
	SaveData->MaxHealth = Character->PlayerStats->MaxHealth;
	SaveData->CurrentRunes = Character->PlayerStats->CurrentRunes;
	SaveData->PlayerLevel = Character->PlayerStats->CurrentLevel;
	SaveData->VigorLevel = Character->PlayerStats->VigorLevel;
	SaveData->MindLevel = Character->PlayerStats->MindLevel;
	SaveData->EnduranceLevel = Character->PlayerStats->EnduranceLevel;
	SaveData->LevelName = GetWorld()->GetName();

	// ?? Save inventory
	if (Character->Inventory)
	{
		SaveData->SavedEquippedWeapons = Character->Inventory->EquippedWeapons;
		SaveData->SavedBackpackWeapons = Character->Inventory->BackpackWeapons;
		SaveData->SavedEquippedIndex = Character->Inventory->EquippedIndex;
	}

	if (UMyGameInstance* GI = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		SaveData->bHasRuneDrop = GI->bHasRuneDrop;
		SaveData->DroppedRuneAmount = GI->DroppedRuneAmount;
		SaveData->DroppedRuneLocation = GI->DroppedRuneLocation;
	}

	// ?? Save the game
	UGameplayStatics::SaveGameToSlot(SaveData, TEXT("BonfireSlot"), 0);
}


void ABonfire::Interact(AALSBaseCharacter* Character) {
	if (!bCanInteract || Character->bIsResting) return;


	Character->bIsResting = true;
	InteractionWidget->SetVisibility(false);
	// Play Sit animation (you'll trigger a montage or anim notify here)
	Character->PlayBonfireSitAnimation(this);
	Character->bIsInvisibleToEnemies = true;
	SaveGameState(Character);

	for (TActorIterator<AALSAIController> It(GetWorld()); It; ++It)
	{
		AALSAIController* AIController = *It;
		if (AIController)
		{
			AIController->ClearTargetIfPlayerInvisible();
		}
	}
}

