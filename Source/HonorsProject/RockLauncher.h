// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RockLauncher.generated.h"

class UProjectileMovementComponent;
UCLASS()
class HONORSPROJECT_API ARockLauncher : public AActor
{
	GENERATED_BODY()
	
public:	
	ARockLauncher();
	
	virtual void BeginPlay() override;
	void FireRock();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Launcher")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Launcher")
	TObjectPtr<USceneComponent> MuzzlePoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Launcher")
	TSubclassOf<AIncomingRock> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Launcher")
	float FireInterval = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Launcher")
	float AimHeightOffset = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Launcher")
	float InitialFireDelay = 0.5f;
	
protected:
	FTimerHandle FireTimerHandle;
	bool bIsFiring = false;
};
