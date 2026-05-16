#pragma once

#include "CoreMinimal.h"
#include "MagicTypes.generated.h"

UENUM(BlueprintType)
enum class EMagicType : uint8
{
	Magic UMETA(DisplayName = "Magic"),
	Fire UMETA(DisplayName = "Fire"),
	Lightning UMETA(DisplayName = "Lightning"),
	Holy UMETA(DisplayName = "Holy")
};
