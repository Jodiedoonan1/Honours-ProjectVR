// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RisingRock.h"
#include "HandRaiseRockAbilityComponent.generated.h"

UENUM(BlueprintType)
enum class EHandRaiseState : uint8
{
	Idle,
	Armed,
	Cooldown
};

USTRUCT()
struct FHandRaiseData
{
	GENERATED_BODY()

	EHandRaiseState State = EHandRaiseState::Idle;
	float CooldownRemaining = 0.f;

	float LastZ = 0.f;
	bool bHasLast = false;

	float ArmedStartZ = 0.f;
	bool bLowReset = false;
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
	
	void UpdateHand(USceneComponent* HandComp, bool bTriggerHeld, bool bButtonHeld, FHandRaiseData& Data, float DeltaTime);
	
	UPROPERTY(EditAnywhere, Category="HandRaise|Refs")
	FComponentReference LeftHandRef;

	UPROPERTY(EditAnywhere, Category="HandRaise|Refs")
	FComponentReference RightHandRef;

	UPROPERTY(EditAnywhere, Category="HandRaise|Refs")
	FComponentReference HeadRef;
	
	UFUNCTION(BlueprintCallable, Category="HandRaiseRock|Input")
	void SetTriggersHeld(bool bLeftTrigger, bool bRightTrigger, bool bLeftButton, bool bRightButton);

	// Targeting
	UPROPERTY(EditAnywhere, Category="HandRaise|Targeting")
	float TargetRockRange = 250.f;

	UPROPERTY(EditAnywhere, Category="HandRaise|Targeting")
	float TargetRockSphereRadius = 12.f;

	UPROPERTY(EditAnywhere, Category="HandRaise|Targeting")
	TEnumAsByte<ECollisionChannel> TargetTraceChannel = ECC_Visibility;

	// Gesture thresholds 
	UPROPERTY(EditAnywhere, Category="HandRaise|Gesture")
	float ArmMaxHeightFromHead = 60.f; 

	UPROPERTY(EditAnywhere, Category="HandRaise|Gesture")
	float RaiseMinDeltaZ = 25.f; 

	UPROPERTY(EditAnywhere, Category="HandRaise|Gesture")
	float RaiseMinUpSpeed = 60.f; 

	UPROPERTY(EditAnywhere, Category="HandRaise|Gesture")
	float Cooldown = 0.35f;

	// Strength mapping (reusing LaunchFromStomp)
	UPROPERTY(EditAnywhere, Category="HandRaise|Strength")
	float MinStrength = 80.f;

	UPROPERTY(EditAnywhere, Category="HandRaise|Strength")
	float MaxStrength = 450.f;

	UPROPERTY(EditAnywhere, Category="HandRaise|Strength")
	float UpSpeedForMaxStrength = 220.f;
	
	// Spawning (when no rock is targeted)
	UPROPERTY(EditAnywhere, Category="HandRaise|Spawn")
	TSubclassOf<ARisingRock> RockClass;

	UPROPERTY(EditAnywhere, Category="HandRaise|Spawn")
	float SpawnForwardDistance = 200.f;

	UPROPERTY(EditAnywhere, Category="HandRaise|Spawn")
	float SpawnDownOffset = 35.f;

	UPROPERTY(EditAnywhere, Category="HandRaise|Spawn")
	float TraceUp = 120.f;

	UPROPERTY(EditAnywhere, Category="HandRaise|Spawn")
	float TraceDown = 400.f;
	
	bool GetGroundInFront(FVector& OutLoc, FRotator& OutRot) const;
	ARisingRock* SpawnRock(const FVector& GroundLoc, const FRotator& Rot, float Strength, float HeadZ) const;
	
private:
	USceneComponent* LeftHand = nullptr;
	USceneComponent* RightHand = nullptr;
	USceneComponent* Head = nullptr;

	bool bLT = false;
	bool bRT = false;
	bool bLB = false;
	bool bRB = false;

	FHandRaiseData LeftData;
	FHandRaiseData RightData;	
};
