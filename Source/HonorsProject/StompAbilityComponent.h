// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
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
    float RestZ = 0.f;
    float LastZ = 0.f;
    float VerticalSpeed = 0.f;
    float CooldownRemaining = 0.f;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HONORSPROJECT_API UStompAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStompAbilityComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Tracking")
	FComponentReference FootLeftRef;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Tracking")
	FComponentReference FootRightRef;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Tracking")
	FComponentReference ForwardRef;
	
	USceneComponent* FootLeft = nullptr;
	USceneComponent* FootRight = nullptr;
	USceneComponent* ForwardReference = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Spawn")
	TSubclassOf<AActor> RockClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Tuning")
	float LiftThreshold = 18.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Tuning")
	float LandTolerance = 8.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Tuning")
	float MinDownSpeed = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Tuning")
	float Cooldown = 0.35f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Spawn")
	float SpawnForwardDistance = 70.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Spawn")
	float SpawnDownOffset = 30.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Spawn")
	float TraceUp = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp|Spawn")
	float TraceDown = 200.f;
	
	UFUNCTION(BlueprintCallable, Category="Stomp")
	void Calibrate();
	
private:
	FFootStompData Left;
	FFootStompData Right;

	void UpdateFoot(USceneComponent* Foot, FFootStompData& Data, float DeltaTime);
	bool GetGroundInFront(FVector& OutLoc, FRotator& OutRot) const;
	void SpawnRock(const FVector& GroundLoc, const FRotator& Rot) const;
};
