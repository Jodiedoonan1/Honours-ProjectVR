// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "LaunchPillar.h"
#include "TrackedVelocity.h"
#include "EarthLaunchAbilityComponent.generated.h"

UENUM()
enum class EEarthLaunchState : uint8
{
	Idle,
	HoldingButtons,
	PrimedDownBack,
	Cooldown
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HONORSPROJECT_API UEarthLaunchAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UEarthLaunchAbilityComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthLaunch|Tracking")
	FComponentReference LeftHandRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthLaunch|Tracking")
	FComponentReference RightHandRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthLaunch|Tracking")
	FComponentReference ForwardRef;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthLaunch|Spawn")
	TSubclassOf<ALaunchPillar> PillarClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthLaunch|Spawn")
	float SpawnBackwardDistance = 0.0f; // This was 160

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthLaunch|Spawn")
	float SpawnDownOffset = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthLaunch|Spawn")
	float TraceUp = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthLaunch|Spawn")
	float TraceDown = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthLaunch|Launch")
	float LaunchUpScaleMin = 0.4f; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthLaunch|Launch")
	float LaunchUpScaleMax = 1.2f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthLaunch|Tuning")
	float DownDotThreshold = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthLaunch|Tuning")
	float BackDotThreshold = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthLaunch|Tuning")
	float ForwardDotThreshold = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthLaunch|Tuning")
	float UpDotThreshold = 0.4f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthLaunch|Tuning")
	float LaunchUpMin = 0.20f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthLaunch|Tuning")
	float LaunchUpMax = 1.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthLaunch|Tuning")
	float Cooldown = 0.35f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthLaunch|Tuning")
	float PrimeTimeout = 0.6f;
	
	UFUNCTION(BlueprintCallable, Category="EarthLaunch|Input")
	void SetTriggersHeld(bool bLeftTrigger, bool bRightTrigger);
	
	UPROPERTY(EditAnywhere, Category="EarthLaunch|Velocity")
	FComponentReference LeftVelocityRef;

	UPROPERTY(EditAnywhere, Category="EarthLaunch|Velocity")
	FComponentReference RightVelocityRef;

	UTrackedVelocity* LeftVel = nullptr;
	UTrackedVelocity* RightVel = nullptr;

	UPROPERTY(EditAnywhere, Category="EarthLaunch|Launch")
	float MinLaunchSpeed = 400.f;

	UPROPERTY(EditAnywhere, Category="EarthLaunch|Launch")
	float MaxLaunchSpeed = 1400.f;

	UPROPERTY(EditAnywhere, Category="EarthLaunch|Launch")
	float MaxHandSpeedForFullLaunch = 250.f;
	
	UFUNCTION(BlueprintCallable, Category="EarthLaunch|Launch")
	float ComputeLaunchPowerT() const;
	
	bool bLaunching = false;
	FVector LaunchVelocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category="EarthLaunch|Launch")
	float LaunchGravity = 980.f;

	UPROPERTY(EditAnywhere, Category="EarthLaunch|Launch")
	float LaunchDrag = 0.0f;

	UPROPERTY(EditAnywhere, Category="EarthLaunch|Launch")
	float GroundFriction = 0.0f;
	
private:
	USceneComponent* LeftHand = nullptr;
	USceneComponent* RightHand = nullptr;
	USceneComponent* ForwardComp = nullptr;

	bool bLT = false, bRT = false;

	EEarthLaunchState State = EEarthLaunchState::Idle;
	float CooldownRemaining = 0.0f;
	float PrimeRemaining = 0.0f;

	bool AreAllButtonsHeld() const { return bLT && bRT; }
	
	bool TryGetGroundPoint(FVector& OutLoc, FRotator& OutRot) const;
	FVector ComputeLaunchDirection(float AvgUpDot) const;
	void ApplyLaunch(float AvgUpDot);
	void SpawnLaunchPillar(const FVector& GroundLoc, const FRotator& FacingRot, const FVector& LaunchDir);
};