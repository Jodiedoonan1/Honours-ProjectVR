// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RisingRock.h"
#include "HandRaiseRockAbilityComponent.generated.h"

enum class EHandRaiseState : uint8
{
	Idle,
	Armed,
	Cooldown
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HONORSPROJECT_API UHandRaiseRockAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHandRaiseRockAbilityComponent();

protected:
	virtual void BeginPlay() override;
	
	ARisingRock* FindLookedAtRock(FHitResult& OutHit) const;
	float ComputeRaiseStrength(float UpSpeed) const;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
