// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community


#include "Character/ALSPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "AI/ALSAIController.h"
#include "Character/ALSCharacter.h"
#include "Character/ALSPlayerCameraManager.h"
#include "Components/ALSDebugComponent.h"
//#include "Bonfire.h"
#include "Kismet/GameplayStatics.h"

void AALSPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);
	PossessedCharacter = Cast<AALSBaseCharacter>(NewPawn);
	if (!IsRunningDedicatedServer())
	{
		// Servers want to setup camera only in listen servers.
		SetupCamera();
	}

	SetupInputs();

	if (!IsValid(PossessedCharacter)) return;

	UALSDebugComponent* DebugComp = Cast<UALSDebugComponent>(PossessedCharacter->GetComponentByClass(UALSDebugComponent::StaticClass()));
	if (DebugComp)
	{
		DebugComp->OnPlayerControllerInitialized(this);
	}
}

void AALSPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
	PossessedCharacter = Cast<AALSBaseCharacter>(GetPawn());
	SetupCamera();
	SetupInputs();

	if (!PossessedCharacter) return;

	UALSDebugComponent* DebugComp = Cast<UALSDebugComponent>(PossessedCharacter->GetComponentByClass(UALSDebugComponent::StaticClass()));
	if (DebugComp)
	{
		DebugComp->OnPlayerControllerInitialized(this);
	}
}

void AALSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->ClearActionEventBindings();
		EnhancedInputComponent->ClearActionValueBindings();
		EnhancedInputComponent->ClearDebugKeyBindings();

		TSet<const UInputAction*> BoundActions;
		BindActions(DefaultInputMappingContext, BoundActions);
		BindActions(DebugInputMappingContext, BoundActions);
	}
	else
	{
		UE_LOG(LogTemp, Fatal, TEXT("ALS Community requires Enhanced Input System to be activated in project settings to function properly"));
	}
}

bool AALSPlayerController::InputKey(const FInputKeyEventArgs& Params)
{
	const bool bHandled = Super::InputKey(Params);
	const FKey Key = Params.Key;

	EInteractionInputType NewInputType = EInteractionInputType::KeyboardMouse;
	if (Key.IsGamepadKey())
	{
		const FString KeyName = Key.GetFName().ToString();
		const bool bIsPlayStationKey =
			KeyName.Contains(TEXT("PlayStation")) ||
			KeyName.Contains(TEXT("PS4")) ||
			KeyName.Contains(TEXT("PS5")) ||
			KeyName.Contains(TEXT("DualShock")) ||
			KeyName.Contains(TEXT("DualSense"));

		NewInputType = bIsPlayStationKey ? EInteractionInputType::PlayStationGamepad : EInteractionInputType::XboxGamepad;
	}

	CurrentInteractionInputType = NewInputType;
	return bHandled;
}

void AALSPlayerController::BindActions(UInputMappingContext* Context, TSet<const UInputAction*>& BoundActions)
{
	if (Context)
	{
		const TArray<FEnhancedActionKeyMapping>& Mappings = Context->GetMappings();
		UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
		if (EnhancedInputComponent)
		{
			for (const FEnhancedActionKeyMapping& Keymapping : Mappings)
			{
				if (!Keymapping.Action || BoundActions.Contains(Keymapping.Action))
				{
					continue;
				}

				BoundActions.Add(Keymapping.Action);
				if (Keymapping.Action->GetFName() == GET_FUNCTION_NAME_CHECKED(AALSPlayerController, JumpAction))
				{
					EnhancedInputComponent->BindAction(Keymapping.Action, ETriggerEvent::Started, this, &AALSPlayerController::JumpStartedAction);
					EnhancedInputComponent->BindAction(Keymapping.Action, ETriggerEvent::Completed, this, &AALSPlayerController::JumpReleasedAction);
					EnhancedInputComponent->BindAction(Keymapping.Action, ETriggerEvent::Canceled, this, &AALSPlayerController::JumpReleasedAction);
				}
				else if (Keymapping.Action->GetFName() == GET_FUNCTION_NAME_CHECKED(AALSPlayerController, HeavyAttackAction))
				{
					EnhancedInputComponent->BindAction(Keymapping.Action, ETriggerEvent::Started, this, &AALSPlayerController::HeavyAttackStartedAction);
					EnhancedInputComponent->BindAction(Keymapping.Action, ETriggerEvent::Completed, this, &AALSPlayerController::HeavyAttackReleasedAction);
					EnhancedInputComponent->BindAction(Keymapping.Action, ETriggerEvent::Canceled, this, &AALSPlayerController::HeavyAttackReleasedAction);
				}
				else if (Keymapping.Action->GetFName() == GET_FUNCTION_NAME_CHECKED(AALSPlayerController, CheckForStanceChangeAction))
				{
					EnhancedInputComponent->BindAction(Keymapping.Action, ETriggerEvent::Started, this, &AALSPlayerController::CheckForStanceChangeStartedAction);
					EnhancedInputComponent->BindAction(Keymapping.Action, ETriggerEvent::Completed, this, &AALSPlayerController::CheckForStanceChangeReleasedAction);
					EnhancedInputComponent->BindAction(Keymapping.Action, ETriggerEvent::Canceled, this, &AALSPlayerController::CheckForStanceChangeReleasedAction);
				}
				else if (Keymapping.Action->GetFName() == GET_FUNCTION_NAME_CHECKED(AALSPlayerController, AimAction))
				{
					EnhancedInputComponent->BindAction(Keymapping.Action, ETriggerEvent::Started, this, &AALSPlayerController::AimAction);
					EnhancedInputComponent->BindAction(Keymapping.Action, ETriggerEvent::Completed, this, &AALSPlayerController::AimAction);
					EnhancedInputComponent->BindAction(Keymapping.Action, ETriggerEvent::Canceled, this, &AALSPlayerController::AimAction);
				}
				else
				{
					EnhancedInputComponent->BindAction(Keymapping.Action, ETriggerEvent::Triggered, Cast<UObject>(this), Keymapping.Action->GetFName());
				}
			}
		}
	}
}

void AALSPlayerController::SetupInputs()
{
	if (PossessedCharacter)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			FModifyContextOptions Options;
			Options.bForceImmediately = 1;
			Subsystem->AddMappingContext(DefaultInputMappingContext, 1, Options);
			UALSDebugComponent* DebugComp = Cast<UALSDebugComponent>(PossessedCharacter->GetComponentByClass(UALSDebugComponent::StaticClass()));
			if (DebugComp)
			{
				// Do only if we have debug component
				Subsystem->AddMappingContext(DebugInputMappingContext, 0, Options);
			}
		}
	}
}

void AALSPlayerController::SetupCamera()
{
	// Call "OnPossess" in Player Camera Manager when possessing a pawn
	AALSPlayerCameraManager* CastedMgr = Cast<AALSPlayerCameraManager>(PlayerCameraManager);
	if (PossessedCharacter && CastedMgr)
	{
		CastedMgr->OnPossess(PossessedCharacter);
	}
}

bool AALSPlayerController::ShouldIgnoreGameplayInput() const
{
	return !PossessedCharacter || (PossessedCharacter->Inventory && PossessedCharacter->Inventory->bHealing);
}

void AALSPlayerController::HandleBonfireVerticalNavigation(float AxisValue)
{
	if (!PossessedCharacter || !PossessedCharacter->CurrentBonfireMenu)
		return;

	if (FMath::Abs(AxisValue) < 0.5f)
	{
		bCanBonfireVerticalNavigate = true;
		return;
	}

	if (!bCanBonfireVerticalNavigate)
		return;

	bCanBonfireVerticalNavigate = false;
	GetWorldTimerManager().SetTimer(BonfireVerticalNavigationTimer, this, &AALSPlayerController::EnableBonfireVerticalNavigation, 0.18f, false);

	const int32 Direction = AxisValue > 0.f ? 0 : 1;
	if (PossessedCharacter->CurrentBonfireMenu->isLevelMenuOpen)
	{
		PossessedCharacter->BonfireSkillsNavigate(Direction);
	}
	else
	{
		PossessedCharacter->BonfireMenuNavigate(Direction);
	}
}

void AALSPlayerController::HandleBonfireHorizontalNavigation(float AxisValue)
{
	if (!PossessedCharacter || !PossessedCharacter->CurrentBonfireMenu)
		return;

	if (FMath::Abs(AxisValue) < 0.5f)
	{
		bCanBonfireHorizontalNavigate = true;
		return;
	}

	if (!bCanBonfireHorizontalNavigate)
		return;

	bCanBonfireHorizontalNavigate = false;
	GetWorldTimerManager().SetTimer(BonfireHorizontalNavigationTimer, this, &AALSPlayerController::EnableBonfireHorizontalNavigation, 0.18f, false);

	if (PossessedCharacter->CurrentBonfireMenu->isLevelMenuOpen)
	{
		const int32 Direction = AxisValue > 0.f ? 1 : -1;
		PossessedCharacter->BonfireSkillIncreaseNavigate(Direction);
	}
}

void AALSPlayerController::EnableBonfireVerticalNavigation()
{
	bCanBonfireVerticalNavigate = true;
}

void AALSPlayerController::EnableBonfireHorizontalNavigation()
{
	bCanBonfireHorizontalNavigate = true;
}

void AALSPlayerController::ForwardMovementAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->PlayerStats->bBeingHit)
		return;

	if (PossessedCharacter->bIsResting)
	{
		HandleBonfireVerticalNavigation(Value.Get<float>());
		return;
	}

	if (PossessedCharacter)
	{
		PossessedCharacter->ForwardMovementAction(Value.GetMagnitude());
	}
}

void AALSPlayerController::RightMovementAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->PlayerStats->bBeingHit)
		return;

	if (PossessedCharacter->bIsResting)
	{
		HandleBonfireHorizontalNavigation(Value.Get<float>());
		return;
	}

	if (PossessedCharacter)
	{
		PossessedCharacter->RightMovementAction(Value.GetMagnitude());
	}
}

void AALSPlayerController::CameraUpAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && PossessedCharacter->bIsResting)
	{
		HandleBonfireVerticalNavigation(Value.Get<float>());
		return;
	}

	if (PossessedCharacter && PossessedCharacter->Inventory && PossessedCharacter->Inventory->bIsInventoryOpen)
		return;

	if (PossessedCharacter)
	{
		PossessedCharacter->CameraUpAction(Value.GetMagnitude());
	}
}

void AALSPlayerController::CameraRightAction(const FInputActionValue& Value)
{
	const float AxisValue = Value.Get<float>();

	if (PossessedCharacter && PossessedCharacter->bIsResting)
	{
		HandleBonfireHorizontalNavigation(AxisValue);
		return;
	}

	if (PossessedCharacter && PossessedCharacter->Inventory && PossessedCharacter->Inventory->bIsInventoryOpen)
		return;

	if (PossessedCharacter)
	{
		PossessedCharacter->CameraRightAction(Value.GetMagnitude());
	}

	if (PossessedCharacter->CameraSystem)
	{
		PossessedCharacter->CameraSystem->OnCameraRightInput(AxisValue);
	}
}

void AALSPlayerController::JumpAction(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		JumpStartedAction(Value);
	}
	else
	{
		JumpReleasedAction(Value);
	}
}

void AALSPlayerController::JumpStartedAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->PlayerStats->bBeingHit)
		return;

	if (PossessedCharacter->bIsResting && !PossessedCharacter->CurrentBonfireMenu->isLevelMenuOpen)
	{
		if (UBonfireMenuWidget* Menu = Cast<UBonfireMenuWidget>(PossessedCharacter->CurrentBonfireMenu))
		{
			Menu->SelectOption();
		}
		return;
	}
	else if (PossessedCharacter->bIsResting && PossessedCharacter->CurrentBonfireMenu->isLevelMenuOpen)
	{
		if (UBonfireMenuWidget* Menu = Cast<UBonfireMenuWidget>(PossessedCharacter->CurrentBonfireMenu))
		{
			if (Menu->bCanConfirmLevelUp)
				Menu->ConfirmLevelUp();
		}
		return;
	}
	if (PossessedCharacter)
	{
		PossessedCharacter->JumpAction(true);
	}
}

void AALSPlayerController::JumpReleasedAction(const FInputActionValue& Value)
{
	if (!PossessedCharacter)
		return;

	if (PossessedCharacter->bIsResting)
		return;

	PossessedCharacter->JumpAction(false);
}

void AALSPlayerController::InteractAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->ActiveLootDrop)
	{
		PossessedCharacter->ActiveLootDrop->GiveLootToPlayer(PossessedCharacter);
	}

	if (PossessedCharacter && PossessedCharacter->bCanInteractWithBonfire)
	{
		PossessedCharacter->ActiveBonfire->Interact(PossessedCharacter);
		return;
	}
}

void AALSPlayerController::WeaponChangeAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->bIsResting)
		return;

	if (PossessedCharacter)
	{
		if (PossessedCharacter->Inventory->bIsInventoryOpen)
			return;

		if (PossessedCharacter->CombatSystem->bIsAttacking)
			return;

		PossessedCharacter->Inventory->CycleNextWeapon();
	}
}

void AALSPlayerController::FlaskChangeAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->bIsResting)
		return;

	if (PossessedCharacter)
	{
		PossessedCharacter->Inventory->TogglePotionType();
	}
}


void AALSPlayerController::UsePotionAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->bIsResting)
		return;

	if (PossessedCharacter)
	{
		if (!PossessedCharacter->Inventory->bHealing)
			PossessedCharacter->Inventory->UseActivePotion();
	}
}

void AALSPlayerController::StanceChangeAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->bIsResting)
		return;

	if (PossessedCharacter)
	{
		PossessedCharacter->CombatSystem->SetStance();
	}
}

void AALSPlayerController::CheckForStanceChangeAction(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		CheckForStanceChangeStartedAction(Value);
	}
	else
	{
		CheckForStanceChangeReleasedAction(Value);
	}
}

void AALSPlayerController::CheckForStanceChangeStartedAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->bIsResting)
		return;

	if (PossessedCharacter)
	{
		PossessedCharacter->CombatSystem->SetCheckingForStanceChange(true);
	}
}

void AALSPlayerController::CheckForStanceChangeReleasedAction(const FInputActionValue& Value)
{
	if (!PossessedCharacter)
		return;

	if (PossessedCharacter->CombatSystem)
	{
		PossessedCharacter->CombatSystem->SetCheckingForStanceChange(false);
	}
}

void AALSPlayerController::LightAttackAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->bIsResting)
		return;

	if (PossessedCharacter->Inventory->bIsInventoryOpen)
	{
		return;
	}

	if (PossessedCharacter)
	{
		PossessedCharacter->CombatSystem->LightAttack();
	}
}

void AALSPlayerController::InventorySelectAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (!PossessedCharacter || !PossessedCharacter->Inventory || !PossessedCharacter->Inventory->bIsInventoryOpen)
		return;

	PossessedCharacter->ConfirmSlot();
}

void AALSPlayerController::AshOfWarAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->bIsResting)
		return;

	if (PossessedCharacter)
	{
		PossessedCharacter->CombatSystem->UseAshOfWar();
		PossessedCharacter->PlayerStats->UseFP(25.0f);
	}
}

void AALSPlayerController::HeavyAttackAction(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		HeavyAttackStartedAction(Value);
	}
	else
	{
		HeavyAttackReleasedAction(Value);
	}
}

void AALSPlayerController::HeavyAttackStartedAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->bIsResting)
		return;

	if (PossessedCharacter)
	{
		PossessedCharacter->CombatSystem->StartChargeHeavyAttack();
	}
}

void AALSPlayerController::HeavyAttackReleasedAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->bIsResting)
		return;

	if (PossessedCharacter)
	{
		PossessedCharacter->CombatSystem->ReleaseChargeHeavyAttack();
	}
}

void AALSPlayerController::CrouchAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->bIsResting)
		return;

	if (PossessedCharacter)
	{
		PossessedCharacter->CrouchAction(Value.Get<bool>());
	}
}

void AALSPlayerController::TargetLockAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->bIsResting)
		return;

	if (PossessedCharacter->CameraSystem)
	{
		PossessedCharacter->CameraSystem->TargetLock();
	}
}

void AALSPlayerController::SprintAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->bIsResting)
		return;

	if (PossessedCharacter)
	{
		PossessedCharacter->SprintAction(Value.Get<bool>());
	}
}

void AALSPlayerController::AimAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->bIsResting)
		return;

	if (PossessedCharacter->CombatSystem && PossessedCharacter->CombatSystem->checkingForStanceChange)
		return;

	if (PossessedCharacter)
	{
		if (PossessedCharacter->CombatSystem)
		{
			if (Value.Get<bool>())
			{
				PossessedCharacter->CombatSystem->StartBowDraw();
			}
			else
			{
				PossessedCharacter->CombatSystem->CancelBowDraw();
			}
		}

		PossessedCharacter->AimAction(Value.Get<bool>());
	}
}

void AALSPlayerController::CameraTapAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->bIsResting)
		return;

	if (PossessedCharacter)
	{
		PossessedCharacter->CameraTapAction();
	}
}

void AALSPlayerController::CameraHeldAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->bIsResting)
		return;

	if (PossessedCharacter)
	{
		PossessedCharacter->CameraHeldAction();
	}
}

void AALSPlayerController::RollAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->PlayerStats->bBeingHit)
		return;

	if (PossessedCharacter->bIsResting && !PossessedCharacter->CurrentBonfireMenu->isLevelMenuOpen)
	{
		PossessedCharacter->ActiveBonfire->ExitBonfire(PossessedCharacter, false);
		return;
	}
	else if (PossessedCharacter->bIsResting && PossessedCharacter->CurrentBonfireMenu->isLevelMenuOpen)
	{
		PossessedCharacter->ActiveBonfire->ExitBonfire(PossessedCharacter, true);
		return;
	}

	if (PossessedCharacter)// && Value.Get<bool>())
	{
		PossessedCharacter->RollAction();
	}
}

void AALSPlayerController::WalkAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->bIsResting)
		return;

	if (PossessedCharacter && Value.Get<bool>())
	{
		PossessedCharacter->WalkAction();
	}
}

void AALSPlayerController::RagdollAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->bIsResting)
		return;

	if (PossessedCharacter && Value.Get<bool>())
	{
		PossessedCharacter->RagdollAction();
	}
}

void AALSPlayerController::VelocityDirectionAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->bIsResting)
		return;

	if (PossessedCharacter && Value.Get<bool>())
	{
		PossessedCharacter->VelocityDirectionAction();
	}
}

void AALSPlayerController::LookingDirectionAction(const FInputActionValue& Value)
{
	if (ShouldIgnoreGameplayInput())
		return;

	if (PossessedCharacter->bIsResting)
		return;

	if (PossessedCharacter && Value.Get<bool>())
	{
		PossessedCharacter->LookingDirectionAction();
	}
}

void AALSPlayerController::DebugToggleHudAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && Value.Get<bool>())
	{
		UALSDebugComponent* DebugComp = Cast<UALSDebugComponent>(PossessedCharacter->GetComponentByClass(UALSDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->ToggleHud();
		}
	}
}

void AALSPlayerController::DebugToggleDebugViewAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && Value.Get<bool>())
	{
		UALSDebugComponent* DebugComp = Cast<UALSDebugComponent>(PossessedCharacter->GetComponentByClass(UALSDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->ToggleDebugView();
		}
	}
}

void AALSPlayerController::DebugToggleTracesAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && Value.Get<bool>())
	{
		UALSDebugComponent* DebugComp = Cast<UALSDebugComponent>(PossessedCharacter->GetComponentByClass(UALSDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->ToggleTraces();
		}
	}
}

void AALSPlayerController::DebugToggleShapesAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && Value.Get<bool>())
	{
		UALSDebugComponent* DebugComp = Cast<UALSDebugComponent>(PossessedCharacter->GetComponentByClass(UALSDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->ToggleDebugShapes();
		}
	}
}

void AALSPlayerController::DebugToggleLayerColorsAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && Value.Get<bool>())
	{
		UALSDebugComponent* DebugComp = Cast<UALSDebugComponent>(PossessedCharacter->GetComponentByClass(UALSDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->ToggleLayerColors();
		}
	}
}

void AALSPlayerController::DebugToggleCharacterInfoAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && Value.Get<bool>())
	{
		UALSDebugComponent* DebugComp = Cast<UALSDebugComponent>(PossessedCharacter->GetComponentByClass(UALSDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->ToggleCharacterInfo();
		}
	}
}

void AALSPlayerController::DebugToggleSlomoAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && Value.Get<bool>())
	{
		UALSDebugComponent* DebugComp = Cast<UALSDebugComponent>(PossessedCharacter->GetComponentByClass(UALSDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->ToggleSlomo();
		}
	}
}

void AALSPlayerController::DebugFocusedCharacterCycleAction(const FInputActionValue& Value)
{
	if (PossessedCharacter)
	{
		UALSDebugComponent* DebugComp = Cast<UALSDebugComponent>(PossessedCharacter->GetComponentByClass(UALSDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->FocusedDebugCharacterCycle(Value.GetMagnitude() > 0);
		}
	}
}

void AALSPlayerController::DebugToggleMeshAction(const FInputActionValue& Value)
{
	if (PossessedCharacter && Value.Get<bool>())
	{
		UALSDebugComponent* DebugComp = Cast<UALSDebugComponent>(PossessedCharacter->GetComponentByClass(UALSDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->ToggleDebugMesh();
		}
	}
}

void AALSPlayerController::DebugOpenOverlayMenuAction(const FInputActionValue& Value)
{
	if (PossessedCharacter)
	{
		UALSDebugComponent* DebugComp = Cast<UALSDebugComponent>(PossessedCharacter->GetComponentByClass(UALSDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->OpenOverlayMenu(Value.Get<bool>());
		}
	}
}

void AALSPlayerController::DebugOverlayMenuCycleAction(const FInputActionValue& Value)
{
	if (PossessedCharacter)
	{
		UALSDebugComponent* DebugComp = Cast<UALSDebugComponent>(PossessedCharacter->GetComponentByClass(UALSDebugComponent::StaticClass()));
		if (DebugComp)
		{
			DebugComp->OverlayMenuCycle(Value.GetMagnitude() > 0);
		}
	}
}
