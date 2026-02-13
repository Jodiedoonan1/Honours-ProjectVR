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
    float VerticalSpeed = 0.0f;
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
	float TargetRockRange = 250.f;

	UPROPERTY(EditAnywhere, Category="Stomp|Targeting")
	float TargetRockSphereRadius = 12.f; 

	UPROPERTY(EditAnywhere, Category="Stomp|Targeting")
	TEnumAsByte<ECollisionChannel> TargetTraceChannel = ECC_Visibility;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Spawn")
	TSubclassOf<AActor> RockClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Tuning")
	float LiftThreshold = 18.00f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Tuning")
	float LandTolerance = 8.00f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Tuning")
	float MinDownSpeed = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Tuning")
	float Cooldown = 0.35f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Spawn")
	float SpawnForwardDistance = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Spawn")
	float SpawnDownOffset = 30.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Spawn")
	float TraceUp = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Spawn")
	float TraceDown = 200.0f;
	
	UFUNCTION(BlueprintCallable, Category="Stomp")
	void Calibrate();
	
	ARisingRock* FindLookedAtRock(FHitResult& OutHit) const;
	
private:
	FFootStompData Left;
	FFootStompData Right;

	void UpdateFoot(USceneComponent* Foot, FFootStompData& Data, float DeltaTime);
	bool GetGroundInFront(FVector& OutLoc, FRotator& OutRot) const;
	void SpawnRock(const FVector& GroundLoc, const FRotator& Rot, float StompStrength) const;
};
