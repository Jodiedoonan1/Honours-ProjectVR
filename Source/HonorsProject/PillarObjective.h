// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "PillarObjective.generated.h"

UCLASS()
class HONORSPROJECT_API APillarObjective : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APillarObjective();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
