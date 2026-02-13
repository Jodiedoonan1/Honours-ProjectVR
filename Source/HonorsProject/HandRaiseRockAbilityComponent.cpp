// Fill out your copyright notice in the Description page of Project Settings.


#include "HandRaiseRockAbilityComponent.h"

UHandRaiseRockAbilityComponent::UHandRaiseRockAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UHandRaiseRockAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UHandRaiseRockAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

