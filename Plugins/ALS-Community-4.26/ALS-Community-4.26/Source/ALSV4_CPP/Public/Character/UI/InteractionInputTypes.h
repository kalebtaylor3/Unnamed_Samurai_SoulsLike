#pragma once

#include "CoreMinimal.h"
#include "InteractionInputTypes.generated.h"

UENUM(BlueprintType)
enum class EInteractionInputType : uint8
{
	KeyboardMouse UMETA(DisplayName = "Keyboard / Mouse"),
	XboxGamepad UMETA(DisplayName = "Xbox Gamepad"),
	PlayStationGamepad UMETA(DisplayName = "PlayStation Gamepad")
};
