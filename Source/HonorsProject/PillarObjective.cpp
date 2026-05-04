// Fill out your copyright notice in the Description page of Project Settings.


#include "PillarObjective.h"
#include "Kismet/GameplayStatics.h"
#include "TaskManager.h"

// Sets default values
APillarObjective::APillarObjective()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APillarObjective::BeginPlay()
{
	Super::BeginPlay();
}