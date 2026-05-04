// Fill out your copyright notice in the Description page of Project Settings.


#include "TaskManager.h"
#include "Blueprint/UserWidget.h"
#include "TaskHUDWidget.h"
#include "RockTarget.h"
#include "ObjectiveMarker.h"
#include "PillarObjective.h"
#include "Engine/TargetPoint.h"
#include "RockLauncherManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATaskManager::ATaskManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATaskManager::BeginPlay()
{
	Super::BeginPlay();

	CurrentProgress = 0;
	bTaskComplete = false;
	CurrentTargetRockSize = ERockSize::Small;
	CurrentTaskStage = ETaskStage::CreateSmallRock;
//	CurrentTaskStage = ETaskStage::RaiseExistingRock;
	CurrentObjectiveText = FText::FromString(TEXT("Stomp the Ground to create a Small Rock.\nRaise a foot as if you're going up a step,\nand stomp with light force."));
//	CurrentObjectiveText = FText::FromString(TEXT("Raise an existing rock"));
	

	if (TaskHUDWidgetClass)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
		if (PlayerController)
		{
			TaskHUDWidgetInstance = CreateWidget<UTaskHUDWidget>(PlayerController, TaskHUDWidgetClass);
			if (TaskHUDWidgetInstance)
			{
				TaskHUDWidgetInstance->SetTaskManager(this);
				TaskHUDWidgetInstance->AddToViewport();
			}
		}
	}

	UpdateHUD();
}

void ATaskManager::NotifyRockCreated(ERockSize RockSize)
{
	if (bTaskComplete)
	{
		return;
	}
	
	if (!IsExpectingStompRock())
	{
		return;
	}

	if (RockSize == CurrentTargetRockSize)
	{
		CurrentProgress++;
		UpdateHUD();

		if (CurrentProgress >= RequiredProgress)
		{
			AdvanceToNextTask();
			return;
		}
	}
}

void ATaskManager::NotifyHandRockCreated()
{
	if (bTaskComplete)
	{
		return;
	}

	if (CurrentTaskStage == ETaskStage::CreateHandRock)
	{
		CurrentProgress++;
		UpdateHUD();

		if (CurrentProgress >= RequiredProgress)
		{
			AdvanceToNextTask();
			return;
		}
	}
}

void ATaskManager::NotifyPunchedRock()
{
	if (bTaskComplete)
	{
		return;
	}
	
	if (CurrentTaskStage == ETaskStage::PunchRock)
	{
		CurrentProgress++;
		UpdateHUD();
		
		if (CurrentProgress >= RequiredProgress)
		{
			AdvanceToNextTask();
			return;
		}
	}
}

void ATaskManager::NotifyKickedRock()
{
	if (bTaskComplete)
	{
		return;
	}
	
	if (CurrentTaskStage == ETaskStage::KickRock)
	{
		CurrentProgress++;
		UpdateHUD();
		
		if (CurrentProgress >= RequiredProgress)
		{
			AdvanceToNextTask();
			return;
		}
	}
}

void ATaskManager::NotifyRaisedExistingRock()
{
	if (bTaskComplete)
	{
		return;
	}
	
	if (CurrentTaskStage == ETaskStage::RaiseExistingRock)
	{
		CurrentProgress++;
		UpdateHUD();
		
		if (CurrentProgress >= RequiredProgress)
		{
			AdvanceToNextTask();
			return;
		}
	}
}

void ATaskManager::NotifyTargetHit()
{
	if (bTaskComplete)
	{
		return;
	}

	if (!IsExpectingTargetTask())
	{
		return;
	}

	CurrentProgress++;
	UpdateHUD();

	if (CurrentProgress >= RequiredProgress)
	{
		AdvanceToNextTask();
		return;
	}

	SpawnNextLaunchTarget();
}

void ATaskManager::NotifyObjectiveMarkerReached()
{
	if (!IsExpectingMoveMarker())
	{
		return;
	}
	AdvanceToNextTask();
}

void ATaskManager::NotifyWallRaised()
{
	if (bTaskComplete)
	{
		return;
	}
	
	if (!IsExpectingWallRaised())
	{
		return;
	}
	
	CurrentProgress++;

	if (CurrentProgress >= RequiredProgress)
	{
		AdvanceToNextTask();
		return;
	}
}

void ATaskManager::NotifyWallLowered()
{
	if (bTaskComplete)
	{
		return;
	}
	
	if (!IsExpectingWallLowered())
	{
		return;
	}
	
	CurrentProgress++;

	if (CurrentProgress >= RequiredProgress)
	{
		AdvanceToNextTask();
		return;
	}
}

void ATaskManager::NotifyWallStrike()
{
	if (bTaskComplete)
	{
		return;
	}
	
	if (!IsExpectingWallStrike())
	{
		return;
	}
	
	CurrentProgress++;

	if (CurrentProgress >= RequiredProgress)
	{
		AdvanceToNextTask();
		return;
	}
}

void ATaskManager::NotifyBlockedRock()
{
	if (bTaskComplete)
	{
		return;
	}
	
	if (!IsExpectingBlockedRock())
	{
		return;
	}
	
	CurrentProgress++;

	if (CurrentProgress >= RequiredProgress)
	{
		if (RockLauncherManager)
		{
			RockLauncherManager->StopFiring();
		}

		AdvanceToNextTask();
	}
}

void ATaskManager::NotifySpawnerDestroyed()
{
	if (bTaskComplete)
	{
		return;
	}
	
	if (!IsExpectingSpawnerDestroyed())
	{
		return;
	}
	
	CurrentProgress++;

	if (CurrentProgress >= RequiredProgress)
	{
		AdvanceToNextTask();
		return;
	}
}

bool ATaskManager::ShouldUseRockColourFeedback() const
{
	return CurrentTaskStage == ETaskStage::CreateSmallRock
		|| CurrentTaskStage == ETaskStage::CreateMediumRock
		|| CurrentTaskStage == ETaskStage::CreateLargeRock
		|| CurrentTaskStage == ETaskStage::CreateHandRock;
}

FText ATaskManager::GetCurrentObjectiveText() const
{
	return CurrentObjectiveText;
}

ERockSize ATaskManager::GetCurrentTargetRockSize() const
{
	return CurrentTargetRockSize;
}

int32 ATaskManager::GetCurrentProgress() const
{
	return CurrentProgress;
}

int32 ATaskManager::GetRequiredProgress() const
{
	return RequiredProgress;
}

bool ATaskManager::IsCurrentTaskComplete() const
{
	return bTaskComplete;
}

bool ATaskManager::IsExpectingStompRock() const
{
	return CurrentTaskStage == ETaskStage::CreateSmallRock
		|| CurrentTaskStage == ETaskStage::CreateMediumRock
		|| CurrentTaskStage == ETaskStage::CreateLargeRock;
}

bool ATaskManager::IsExpectingHandRock() const
{
	return CurrentTaskStage == ETaskStage::CreateHandRock;
}

bool ATaskManager::IsExpectingPunchRock() const
{
	return CurrentTaskStage == ETaskStage::PunchRock;
}

bool ATaskManager::IsExpectingKickRock() const
{
	return CurrentTaskStage == ETaskStage::KickRock;
}

bool ATaskManager::IsExpectingRaisedExistingRock() const
{
	return CurrentTaskStage == ETaskStage::RaiseExistingRock;
}

bool ATaskManager::IsExpectingTargetTask() const
{
	return CurrentTaskStage == ETaskStage::HitTargets;
}

bool ATaskManager::IsExpectingMoveMarker() const
{
	return CurrentTaskStage == ETaskStage::GoToNextObjective;
}

bool ATaskManager::IsExpectingWallRaised()
{
	return CurrentTaskStage == ETaskStage::RaiseWall;
}

bool ATaskManager::IsExpectingWallLowered()
{
	return CurrentTaskStage == ETaskStage::LowerWall;
}

bool ATaskManager::IsExpectingWallStrike()
{
	return CurrentTaskStage == ETaskStage::StrikeWall;
}

bool ATaskManager::IsExpectingBlockedRock()
{
	return CurrentTaskStage == ETaskStage::BlockRocks;
}

bool ATaskManager::IsExpectingSpawnerDestroyed()
{
	return CurrentTaskStage == ETaskStage::DestroySpawners;
}

void ATaskManager::UpdateHUD()
{
	UE_LOG(LogTemp, Log, TEXT("Objective: %s | Progress: %d / %d"),
		*CurrentObjectiveText.ToString(),
		CurrentProgress,
		RequiredProgress);
}

void ATaskManager::SetRockObjective(ETaskStage NewStage, ERockSize NewRockSize, const FText& NewObjectiveText)
{
	CurrentTaskStage = NewStage;
	CurrentTargetRockSize = NewRockSize;
	CurrentObjectiveText = NewObjectiveText;
	CurrentProgress = 0;
	RequiredProgress = 3;

	UE_LOG(LogTemp, Warning, TEXT("New Objective: %s"), *CurrentObjectiveText.ToString());

	UpdateHUD();
}

void ATaskManager::SetHandRockObjective(const FText& NewObjectiveText)
{
	CurrentTaskStage = ETaskStage::CreateHandRock;
	CurrentObjectiveText = NewObjectiveText;
	CurrentProgress = 0;
	RequiredProgress = 3;

	UE_LOG(LogTemp, Warning, TEXT("New Objective: %s"), *CurrentObjectiveText.ToString());
	UpdateHUD();
}

void ATaskManager::SetPunchObjetive(const FText& NewObjectiveText)
{
	CurrentTaskStage = ETaskStage::PunchRock;
	CurrentObjectiveText = NewObjectiveText;
	CurrentProgress = 0;
	RequiredProgress = 3;
	
	UE_LOG(LogTemp, Warning, TEXT("New Objective: %s"), *CurrentObjectiveText.ToString());
	UpdateHUD();
}

void ATaskManager::SetKickObjective(const FText& NewObjectiveText)
{
	CurrentTaskStage = ETaskStage::KickRock;
	CurrentObjectiveText = NewObjectiveText;
	CurrentProgress = 0;
	RequiredProgress = 3;
	
	UE_LOG(LogTemp, Warning, TEXT("New Objective: %s"), *CurrentObjectiveText.ToString());
	UpdateHUD();
}

void ATaskManager::SetRaiseExistingRockObjective(const FText& NewObjectiveText)
{
	CurrentTaskStage = ETaskStage::RaiseExistingRock;
	CurrentObjectiveText = NewObjectiveText;
	CurrentProgress = 0;
	RequiredProgress = 3;
	
	UE_LOG(LogTemp, Warning, TEXT("New Objective: %s"), *CurrentObjectiveText.ToString());
	UpdateHUD();
}

void ATaskManager::SetRockTargetObjective(const FText& NewObjectiveText)
{
	CurrentTaskStage = ETaskStage::HitTargets;
	CurrentObjectiveText = NewObjectiveText;
	CurrentProgress = 0;
	
	UE_LOG(LogTemp, Warning, TEXT("New Objective: %s"), *CurrentObjectiveText.ToString());
	UpdateHUD();
	
	SpawnNextLaunchTarget();
}

void ATaskManager::SetMarkerObjective(const FText& NewObjectiveText)
{
	CurrentTaskStage = ETaskStage::GoToNextObjective;
	CurrentObjectiveText = NewObjectiveText;
	CurrentProgress = 0;
	
	UE_LOG(LogTemp, Warning, TEXT("New Objective: %s"), *CurrentObjectiveText.ToString());
	UpdateHUD();
	
	SpawnMoveMarker();
}

void ATaskManager::SetWallRaisedObjective(const FText& NewObjectiveText)
{
	CurrentTaskStage = ETaskStage::RaiseWall;
	CurrentObjectiveText = NewObjectiveText;
	CurrentProgress = 0;
	
	UE_LOG(LogTemp, Warning, TEXT("New Objective: %s"), *CurrentObjectiveText.ToString());
	UpdateHUD();
}

void ATaskManager::SetWallLoweredObjective(const FText& NewObjectiveText)
{
	CurrentTaskStage = ETaskStage::LowerWall;
	CurrentObjectiveText = NewObjectiveText;
	CurrentProgress = 0;
	
	UE_LOG(LogTemp, Warning, TEXT("New Objective: %s"), *CurrentObjectiveText.ToString());
	UpdateHUD();
}

void ATaskManager::SetWallStrikeObjective(const FText& NewObjectiveText)
{
	CurrentTaskStage = ETaskStage::StrikeWall;
	CurrentObjectiveText = NewObjectiveText;
	CurrentProgress = 0;
	RequiredProgress = 3;
	
	UE_LOG(LogTemp, Warning, TEXT("New Objective: %s"), *CurrentObjectiveText.ToString());
	UpdateHUD();
}

void ATaskManager::SetBlockedRockObjective(const FText& NewObjectiveText)
{
	CurrentTaskStage = ETaskStage::BlockRocks;
	CurrentObjectiveText = NewObjectiveText;
	CurrentProgress = 0;
	RequiredProgress = 5;
	
	UE_LOG(LogTemp, Warning, TEXT("New Objective: %s"), *CurrentObjectiveText.ToString());
	UpdateHUD();
	
	if (RockLauncherManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: RockLauncherManager is valid"), *GetName());
		RockLauncherManager->StartFiring();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: RockLauncherManager is NULL"), *GetName());
	}
	SpawnRockSpawner();
}

void ATaskManager::SetDestroySpawnerObjective(const FText& NewObjectiveText)
{
	CurrentTaskStage = ETaskStage::DestroySpawners;
	CurrentObjectiveText = NewObjectiveText;
	CurrentProgress = 0;
	
	UE_LOG(LogTemp, Warning, TEXT("New Objective: %s"), *CurrentObjectiveText.ToString());
	UpdateHUD();
	
	if (RockLauncherManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: RockLauncherManager is valid"), *GetName());
		RockLauncherManager->StartFiring();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: RockLauncherManager is NULL"), *GetName());
	}
}

void ATaskManager::SetComplete(const FText& NewObjectiveText)
{
	CurrentTaskStage = ETaskStage::Complete;
	CurrentObjectiveText = NewObjectiveText;
	CurrentProgress = 0;
	
	UE_LOG(LogTemp, Warning, TEXT("New Objective: %s"), *CurrentObjectiveText.ToString());
	UpdateHUD();
}

void ATaskManager::AdvanceToNextTask()
{
	if (CurrentTaskStage == ETaskStage::CreateSmallRock)
	{
		SetRockObjective(
			ETaskStage::CreateMediumRock,
			ERockSize::Medium,
			FText::FromString(TEXT("Stomp the ground to create a medium rock.\nRaise your foot higher,\nand stomp with moderate force."))
		);
	}
	else if (CurrentTaskStage == ETaskStage::CreateMediumRock)
	{
		SetRockObjective(
			ETaskStage::CreateLargeRock,
			ERockSize::Large,
			FText::FromString(TEXT("Stomp the ground to create a large rock.\nRaise your foot HIGHER,\nand stomp with great force."))
		);
	}
	else if (CurrentTaskStage == ETaskStage::CreateLargeRock)
	{
		SetHandRockObjective(
			FText::FromString(TEXT("Raise your hand to create a rock.\nHolding the trigger + side buttons,\nand lift your hand from waist to shoulder."))
		);
	}
	else if (CurrentTaskStage == ETaskStage::CreateHandRock)
	{
		SetPunchObjetive(
			FText::FromString(TEXT("Create a rock, then punch it to launch it. \nStronger punches send it further"))
			);
	}
	else if (CurrentTaskStage == ETaskStage::PunchRock)
	{
		SetKickObjective(
			FText::FromString(TEXT("Create a rock, then kick it to launch it. \nStronger kicks send it further"))
			);
	}
	else if (CurrentTaskStage == ETaskStage::KickRock)
	{
		RequiredProgress = 1;
		CurrentMoveMarkerIndex = 0;
		SetMarkerObjective(
		FText::FromString(TEXT("Go to the marked objective"))
			);
	}
	else if (CurrentTaskStage == ETaskStage::GoToNextObjective && CurrentMoveMarkerIndex == 0)
	{
		RequiredProgress = 5;
		SetRaiseExistingRockObjective(
			FText::FromString(TEXT("Raise an existing rock.\nLook at an existing rock,\nand then raise it with a stomp or with your hand"))
			);
	}
	else if (CurrentTaskStage == ETaskStage::RaiseExistingRock)
	{
		RequiredProgress = 1;
		CurrentMoveMarkerIndex = 1;
		SetMarkerObjective(
		FText::FromString(TEXT("Go to the marked objective"))
	);
	}
	else if (CurrentTaskStage == ETaskStage::GoToNextObjective && CurrentMoveMarkerIndex == 1)
	{
		RequiredProgress = 9;
		
		SetRockTargetObjective(
		FText::FromString(TEXT("Launch rocks at the targets.\nStronger strikes can help you cover more distance"))
		);
	}
	else if (CurrentTaskStage == ETaskStage::HitTargets)
	{
		RequiredProgress = 1;
		
		SetWallRaisedObjective(
			FText::FromString(TEXT("Raise a rock wall.\nHolding the side buttons, raise both hands \nabove your head to raise a wall"))
		);
		
	}
	else if (CurrentTaskStage == ETaskStage::RaiseWall)
	{
		SetWallLoweredObjective(
			FText::FromString(TEXT("Lower a rock wall.\nHolding the side buttons, lower your hands from \nabove your head to your waist to lower the wall."))
		);
	}
	else if (CurrentTaskStage == ETaskStage::LowerWall)
	{
		SetWallStrikeObjective(
			FText::FromString(TEXT("Launch a rock wall.\nRaise a wall and then strike it with a punch or kick"))
		);
	}
	else if (CurrentTaskStage == ETaskStage::StrikeWall)
	{
		CurrentMoveMarkerIndex = 2;

		SetMarkerObjective(
			FText::FromString(TEXT("Go to the next marked objective"))
		);
	}
	else if (CurrentTaskStage == ETaskStage::GoToNextObjective && CurrentMoveMarkerIndex == 2)
	{
		CurrentMoveMarkerIndex = 3;
		SetBlockedRockObjective(
			FText::FromString(TEXT("Countering projectiles.\nUse your wall to defend yourself from incoming rocks."))
		);
	}
	else if (CurrentTaskStage == ETaskStage::BlockRocks)
	{
		RequiredProgress = 3;
		
		SetDestroySpawnerObjective(
			FText::FromString(TEXT("Destroy rock spawn points.\nDestroy the rock spawn points by launching rocks/walls at them.\nWithout getting hit by the rocks"))
		);
	}
	else if (CurrentTaskStage == ETaskStage::DestroySpawners)
	{
		SetComplete(
			FText::FromString(TEXT("All Tasks Complete"))
		);
	}
}

void ATaskManager::CompleteCurrentTask()
{
	bTaskComplete = true;
	UE_LOG(LogTemp, Warning, TEXT("Task Complete: %s"), *CurrentObjectiveText.ToString());

	UpdateHUD();
}

void ATaskManager::PlayerHitReset()
{
	CurrentProgress = 0;
	UpdateHUD();
	SpawnRockSpawner();
}

void ATaskManager::SpawnNextLaunchTarget()
{
	if (RockTargetPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No RockTargetPoints assigned"));
		return;
	}

	if (CurrentSpawnedTarget)
	{
		CurrentSpawnedTarget->Destroy();
		CurrentSpawnedTarget = nullptr;
	}

	ATargetPoint* SpawnPoint = RockTargetPoints[CurrentProgress];
	if (!SpawnPoint)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CurrentSpawnedTarget = GetWorld()->SpawnActor<ARockTarget>(
		RockTargetClass,
		SpawnPoint->GetActorLocation(),
		SpawnPoint->GetActorRotation(),
		Params
	);
}

void ATaskManager::SpawnRockSpawner()
{
	if (RockSpawnerPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No RockSpawnerPoints assigned"));
		return;
	}

	for (ARockTarget* Target : SpawnedTargets)
	{
		if (Target)
		{
			Target->Destroy();
		}
	}
	SpawnedTargets.Empty();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (ATargetPoint* SpawnPoint : RockSpawnerPoints)
	{
		if (!SpawnPoint)
		{
			continue;
		}

		ARockTarget* NewTarget = GetWorld()->SpawnActor<ARockTarget>(
			RockTargetClass,
			SpawnPoint->GetActorLocation(),
			SpawnPoint->GetActorRotation(),
			Params
		);

		if (NewTarget)
		{
			SpawnedTargets.Add(NewTarget);
		}
	}
}

void ATaskManager::SpawnMoveMarker()
{
	if (!MoveMarkerClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveMarkerClass is not set"));
		return;
	}

	if (MoveMarkerPoint.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No MoveMarkerPoints assigned"));
		return;
	}

	if (!MoveMarkerPoint.IsValidIndex(CurrentMoveMarkerIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentMoveMarkerIndex %d is out of range"), CurrentMoveMarkerIndex);
		return;
	}

	if (CurrentMoveMarker)
	{
		CurrentMoveMarker->Destroy();
		CurrentMoveMarker = nullptr;
	}

	ATargetPoint* SpawnPoint = MoveMarkerPoint[CurrentMoveMarkerIndex];
	if (!SpawnPoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveMarkerPoints[%d] is null"), CurrentMoveMarkerIndex);
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CurrentMoveMarker = GetWorld()->SpawnActor<AObjectiveMarker>(
		MoveMarkerClass,
		SpawnPoint->GetActorLocation(),
		SpawnPoint->GetActorRotation(),
		Params
	);
}