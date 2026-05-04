// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "ObjectiveMarker.generated.h"

UCLASS()
class HONORSPROJECT_API AObjectiveMarker : public AActor
{
	GENERATED_BODY()
	
public:	
	AObjectiveMarker();

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnMarkerOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

public:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDecalComponent> Decal;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> TriggerBox;

protected:
	bool bAlreadyTriggered = false;

};
