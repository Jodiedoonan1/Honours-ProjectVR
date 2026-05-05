#pragma once

#include "CoreMinimal.h"
#include "TaskStage.generated.h"

UENUM(BlueprintType)
enum class ETaskStage : uint8
{
	CreateSmallRock UMETA(DisplayName = "Create Small Rock"),
	CreateMediumRock UMETA(DisplayName = "Create Medium Rock"),
	CreateLargeRock UMETA(DisplayName = "Create Large Rock"),
	CreateHandRock UMETA(DisplayName = "Create Hand Rock"),
	PunchRock UMETA(DisplayName = "Punch Rock"),
	KickRock UMETA(DisplayName = "Kick Rock"),
	GoToNextObjective UMETA(DisplayName = "Go to Next Objective"),
	RaiseExistingRock UMETA(DisplayName = "Raise Existing Rock"),
	HitTargets UMETA(DisplayName = "Hit Targets"),
	RaiseWall UMETA(DisplayName = "Raise Wall"),
	LowerWall UMETA(DisplayName = "Lower Wall"),
	StrikeWall UMETA(DisplayName = "Strike Wall"),
	BlockRocks UMETA(DisplayName = "Block Rocks"),
	DestroySpawners UMETA(DisplayName = "Destroy Spawners"),
	FreePlay UMETA(DisplayName = "FreePlay"),
	Complete UMETA(DisplayName = "Complete")
};