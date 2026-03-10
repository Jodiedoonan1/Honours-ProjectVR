// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "RisingRock.h"
#include "StompAbilityComponent.generated.h"
class UMotionControllerComponent;

UENUM()
enum class EFootState : uint8
{
    Grounded,
    Lifted,
    Falling,
    Cooldown
};

USTRUCT()
struct FFootStompData
{
    GENERATED_BODY()

    EFootState State = EFootState::Grounded;
    float RestZ = 0.0f;
    float LastZ = 0.0f;
	float MaxAboveRest = 0.f;
    float VerticalSpeed = 0.0f;
	float PeakDownSpeedNearGround = 0.0f;
	bool bEnteredNearGroundWindow = false;
    float CooldownRemaining = 0.0f;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HONORSPROJECT_API UStompAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UStompAbilityComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Tracking")
	FComponentReference FootLeftRef;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Tracking")
	FComponentReference FootRightRef;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Tracking")
	FComponentReference ForwardRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Tracking")
	FComponentReference HeadRef;
	
	USceneComponent* FootLeft = nullptr;
	USceneComponent* FootRight = nullptr;
	USceneComponent* ForwardReference = nullptr;
	USceneComponent* HeadComponent = nullptr;
	
	UPROPERTY(EditAnywhere, Category="Stomp|Targeting")
	float TargetRockRange = 700.f;

	UPROPERTY(EditAnywhere, Category="Stomp|Targeting")
	float TargetRockSphereRadius = 30.f; 

	UPROPERTY(EditAnywhere, Category="Stomp|Targeting")
	TEnumAsByte<ECollisionChannel> TargetTraceChannel = ECC_Visibility;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Spawn")
	TSubclassOf<AActor> RockClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Tuning")
	float LiftThreshold = 22.00f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Tuning")
	float LandTolerance = 8.00f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Tuning")
	float MinDownSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Tuning")
	float Cooldown = 0.35f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Spawn")
	float SpawnForwardDistance = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Spawn")
	float SpawnDownOffset = 30.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Spawn")
	float TraceUp = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Spawn")
	float TraceDown = 200.0f;
	
	UPROPERTY(EditAnywhere, Category="Stomp|RockSize")
	float MinRockScale = 0.06f;

	UPROPERTY(EditAnywhere, Category="Stomp|RockSize")
	float MaxRockScale = 0.3f;
	
	UPROPERTY(EditAnywhere, Category="Stomp|Size")
	float SizeCurvePower = 1.7f;
	
	UPROPERTY(EditAnywhere, Category="Stomp|RockSize")
	float MaxLiftForFullSize = 70.0f; 
	
	UPROPERTY(EditAnywhere, Category="Stomp|RockSize")
	float MaxDownSpeedForFullSize = 600.0f; 

	// How much lift vs stomp speed matters
	UPROPERTY(EditAnywhere, Category="Stomp|RockSize")
	float LiftWeight = 0.6f;

	UPROPERTY(EditAnywhere, Category="Stomp|RockSize")
	float SpeedWeight = 0.4f;
	
	UPROPERTY(EditAnywhere, Category="Stomp|RockSize")
	float NearGroundWindow = 25.f;
	
	UFUNCTION(BlueprintCallable, Category="Stomp")
	void Calibrate();
	
	ARisingRock* FindLookedAtRock(FHitResult& OutHit) const;
	
private:
	FFootStompData Left;
	FFootStompData Right;

	void UpdateFoot(USceneComponent* Foot, FFootStompData& Data, float DeltaTime);
	bool GetGroundInFront(FVector& OutLoc, FRotator& OutRot) const;
	void SpawnRock(const FVector& GroundLoc, const FRotator& Rot, float StompStrength, float RockScale) const;
};
