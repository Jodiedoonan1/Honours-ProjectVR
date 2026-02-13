// Fill out your copyright notice in the Description page of Project Settings.


#include "TrackedVelocity.h"

UTrackedVelocity::UTrackedVelocity()
{

	PrimaryComponentTick.bCanEverTick = true;
}


void UTrackedVelocity::BeginPlay()
{
	Super::BeginPlay();
	
	AActor* Owner = GetOwner();
	if (!Owner) return;

	Target = Cast<USceneComponent>(TargetRef.GetComponent(Owner));
	if (!Target)
	{
		Target = Owner->GetRootComponent();
	}

	if (Target)
	{
		LastPos = Target->GetComponentLocation();
		bHasLast = true;
	}
	
}

void UTrackedVelocity::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Target || DeltaTime <= KINDA_SMALL_NUMBER) return;

	const FVector CurrentPos = Target->GetComponentLocation();

	if (!bHasLast)
	{
		LastPos = CurrentPos;
		bHasLast = true;
		return;
	}

	RawVelocity = (CurrentPos - LastPos) / DeltaTime;

	LastPos = CurrentPos;
}

