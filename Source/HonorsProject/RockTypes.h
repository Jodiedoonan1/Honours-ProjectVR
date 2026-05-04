#pragma once

#include "CoreMinimal.h"
#include "RockTypes.generated.h"

UENUM(BlueprintType)
enum class ERockSize : uint8
{
	Small	UMETA(DisplayName = "Small"),
	Medium	UMETA(DisplayName = "Medium"),
	Large	UMETA(DisplayName = "Large")
};