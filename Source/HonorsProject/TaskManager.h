// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RockTypes.h"
#include "TaskStage.h"
#include "TaskManager.generated.h"

class UTaskHUDWidget;
class ARockTarget;
class ATargetPoint;
class AObjectiveMarker;
class ARockLauncher;
class ARockLauncherManager;

UCLASS()
class HONORSPROJECT_API ATaskManager : public AActor
{
	GENERATED_BODY()
	
public:	
	
	ATaskManager();

protected:
	
	virtual void BeginPlay() override;

public:	

	UFUNCTION(BlueprintCallable)
	void NotifyRockCreated(ERockSize RockSize);
	
	UFUNCTION(BlueprintCallable)
	void NotifyHandRockCreated();
	
	UFUNCTION(BlueprintCallable)
	void NotifyPunchedRock();
	
	UFUNCTION(BlueprintCallable)
	void NotifyKickedRock();
	
	UFUNCTION(BlueprintCallable)
	void NotifyRaisedExistingRock();
	
	UFUNCTION(BlueprintCallable)
	void NotifyTargetHit();
	
	UFUNCTION(BlueprintCallable)
	void NotifyObjectiveMarkerReached();
	
	UFUNCTION(BlueprintCallable)
	void NotifyWallRaised();
	
	UFUNCTION(BlueprintCallable)
	void NotifyWallLowered();
	
	UFUNCTION(BlueprintCallable)
	void NotifyWallStrike();
	
	UFUNCTION(BlueprintCallable)
	void NotifyBlockedRock();
	
	UFUNCTION(BlueprintCallable)
	void NotifySpawnerDestroyed();

	UFUNCTION(BlueprintCallable)
	FText GetCurrentObjectiveText() const;

	UFUNCTION(BlueprintCallable)
	int32 GetCurrentProgress() const;
	
	UFUNCTION(BlueprintCallable)
	ERockSize GetCurrentTargetRockSize() const;

	UFUNCTION(BlueprintCallable)
	int32 GetRequiredProgress() const;

	UFUNCTION(BlueprintCallable)
	bool IsCurrentTaskComplete() const;
	
	UFUNCTION(BlueprintCallable)
	bool IsExpectingStompRock() const;

	UFUNCTION(BlueprintCallable)
	bool IsExpectingHandRock() const;
	
	UFUNCTION(BlueprintCallable)
	bool IsExpectingPunchRock() const;
	
	UFUNCTION(BlueprintCallable)
	bool IsExpectingKickRock() const;
	
	UFUNCTION(BlueprintCallable)
	bool IsExpectingRaisedExistingRock() const;
	
	UFUNCTION(BlueprintCallable)
	bool IsExpectingTargetTask() const;
	
	UFUNCTION(BlueprintCallable)
	bool IsExpectingMoveMarker() const;
	
	UFUNCTION(BlueprintCallable)
	bool IsExpectingWallRaised();
	
	UFUNCTION(BlueprintCallable)
	bool IsExpectingWallLowered();
	
	UFUNCTION(BlueprintCallable)
	bool IsExpectingWallStrike();
	
	UFUNCTION(BlueprintCallable)
	bool IsExpectingBlockedRock();
	
	UFUNCTION(BlueprintCallable)
	bool IsExpectingSpawnerDestroyed();
	
	UFUNCTION(BlueprintCallable)
	bool ShouldUseRockColourFeedback() const;
	
	void AdvanceToNextTask();
	
	void SetRockObjective(ETaskStage NewStage, ERockSize NewRockSize, const FText& NewObjectiveText);
	void SetHandRockObjective(const FText& NewObjectiveText);
	void SetPunchObjetive(const FText& NewObjectiveText);
	void SetKickObjective(const FText& NewObjectiveText);
	void SetRaiseExistingRockObjective(const FText& NewObjectiveText);
	void SetRockTargetObjective(const FText& NewObjectiveText);
	void SetMarkerObjective(const FText& NewObjectiveText);
	void SetWallRaisedObjective(const FText& NewObjectiveText);
	void SetWallLoweredObjective(const FText& NewObjectiveText);
	void SetWallStrikeObjective(const FText& NewObjectiveText);
	void SetBlockedRockObjective(const FText& NewObjectiveText);
	void SetDestroySpawnerObjective(const FText& NewObjectiveText);
	void SetFreePlay(const FText& NewObjectiveText);
	void SetComplete(const FText& NewObjectiveText);
	
	void PlayerHitReset();
	
	UPROPERTY()
	UTaskHUDWidget* TaskHUDWidgetInstance = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "Targets")
	TSubclassOf<ARockTarget> RockTargetClass;
	
	UPROPERTY(EditAnywhere, Category = "Move Objective")
	TSubclassOf<AObjectiveMarker> MoveMarkerClass;
	
	UPROPERTY(EditAnywhere, Category = "Targets")
	TArray<TObjectPtr<ATargetPoint>> RockTargetPoints;
	
	UPROPERTY(EditAnywhere, Category = "Move Objective")
	TArray<TObjectPtr<ATargetPoint>> MoveMarkerPoint;
	
	UPROPERTY(EditAnywhere, Category = "Targets")
	TArray<TObjectPtr<ATargetPoint>> RockSpawnerPoints;
	
	UPROPERTY()
	TArray<TObjectPtr<ARockTarget>> SpawnedTargets;
	
	UPROPERTY(EditAnywhere, Category = "Block Task")
	TObjectPtr<ARockLauncher> RockLauncher;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Block Task")
	TObjectPtr<ARockLauncherManager> RockLauncherManager;
	
	UPROPERTY()
	TObjectPtr<ARockTarget> CurrentSpawnedTarget = nullptr;

	UPROPERTY()
	TObjectPtr<AObjectiveMarker> CurrentMoveMarker = nullptr;
	
protected:
	void UpdateHUD();
	void CompleteCurrentTask();
	void SpawnNextLaunchTarget();
	void SpawnRockSpawner();
	void SpawnMoveMarker();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task")
	FText CurrentObjectiveText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task")
	int32 RequiredProgress = 2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Task")
	int32 CurrentProgress = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Task")
	bool bTaskComplete = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Task")
	ERockSize CurrentTargetRockSize = ERockSize::Small;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Task")
	ETaskStage CurrentTaskStage = ETaskStage::RaiseExistingRock;
	
	UPROPERTY(VisibleAnywhere, Category = "Move Objective")
	int32 CurrentMoveMarkerIndex = 0;
	
	UPROPERTY(VisibleAnywhere, Category = "Pillar Objective")
	int32 CurrentPillarIndex = 0;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UTaskHUDWidget> TaskHUDWidgetClass;
};