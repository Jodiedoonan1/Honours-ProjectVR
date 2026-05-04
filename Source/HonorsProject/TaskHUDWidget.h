// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TaskHUDWidget.generated.h"

class ATaskManager;

UCLASS()
class HONORSPROJECT_API UTaskHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetTaskManager(ATaskManager* InTaskManager);

	UFUNCTION(BlueprintCallable)
	FText GetObjectiveText() const;

	UFUNCTION(BlueprintCallable)
	FText GetProgressText() const;

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ATaskManager> TaskManager;
};
