// Fill out your copyright notice in the Description page of Project Settings.


#include "RockLauncherManager.h"
#include "RockLauncher.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ARockLauncherManager::ARockLauncherManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ARockLauncherManager::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARockLauncher::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		ARockLauncher* Launcher = Cast<ARockLauncher>(Actor);
		if (Launcher)
		{
			Launchers.Add(Launcher);
		}
	}
}

void ARockLauncherManager::StartFiring()
{
	if (bIsFiring || Launchers.Num() == 0)
	{
		return;
	}

	bIsFiring = true;

	GetWorldTimerManager().SetTimer(
		FireTimerHandle,
		this,
		&ARockLauncherManager::FireFromRandomLauncher,
		FireInterval,
		true,
		InitialFireDelay
	);
}

void ARockLauncherManager::StopFiring()
{
	bIsFiring = false;
	GetWorldTimerManager().ClearTimer(FireTimerHandle);
}

void ARockLauncherManager::FireFromRandomLauncher()
{
	if (Launchers.Num() == 0)
	{
		return;
	}

	const int32 RandomIndex = FMath::RandRange(0, Launchers.Num() - 1);
	ARockLauncher* ChosenLauncher = Launchers[RandomIndex];
	
	if (ChosenLauncher)
	{
		ChosenLauncher->FireRock();
	}
}