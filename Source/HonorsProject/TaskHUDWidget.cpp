// Fill out your copyright notice in the Description page of Project Settings.

#include "TaskHUDWidget.h"
#include "TaskManager.h"

void UTaskHUDWidget::SetTaskManager(ATaskManager* InTaskManager)
{
	TaskManager = InTaskManager;
}

FText UTaskHUDWidget::GetObjectiveText() const
{
	if (TaskManager)
	{
		return TaskManager->GetCurrentObjectiveText();
	}

	return FText::FromString(TEXT("No Objective"));
}

FText UTaskHUDWidget::GetProgressText() const
{
	if (TaskManager)
	{
		const FString ProgressString = FString::Printf(
			TEXT("%d / %d"),
			TaskManager->GetCurrentProgress(),
			TaskManager->GetRequiredProgress());

		return FText::FromString(ProgressString);
	}

	return FText::FromString(TEXT("0 / 0"));
}