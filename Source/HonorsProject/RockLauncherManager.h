// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RockLauncherManager.generated.h"

UCLASS()
class HONORSPROJECT_API ARockLauncherManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ARockLauncherManager();

	void StartFiring();
	void StopFiring();
	
	void FireFromRandomLauncher();
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Firing")
	float FireInterval = 4.5f;

	UPROPERTY(EditAnywhere, Category = "Firing")
	float InitialFireDelay = 0.5f;

	UPROPERTY()
	TArray<ARockLauncher*> Launchers;

	bool bIsFiring = false;
	FTimerHandle FireTimerHandle;
};
