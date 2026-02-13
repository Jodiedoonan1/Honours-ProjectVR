// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "RisingWall.h"
#include "EarthWallAbilityComponent.generated.h"

UENUM()
enum class EEarthWallState : uint8
{
	Idle,
	HoldingButtons,
	PrimedDown,
	PrimedUp,
	CastedCooldown
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HONORSPROJECT_API UEarthWallAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEarthWallAbilityComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthWall|Tracking")
	FComponentReference LeftHandRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthWall|Tracking")
	FComponentReference RightHandRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthWall|Tracking")
	FComponentReference ForwardRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthWall|Spawn")
	TSubclassOf<ARisingWall> WallClass;

	// Tuning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthWall|Tuning")
	float DownDotThreshold = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthWall|Tuning")
	float UpDotThreshold = 0.4f;  

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthWall|Tuning")
	float Cooldown = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthWall|Spawn")
	float SpawnForwardDistance = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthWall|Spawn")
	float SpawnDownOffset = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthWall|Spawn")
	float TraceUp = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EarthWall|Spawn")
	float TraceDown = 300.f;
	
	UPROPERTY()
	TArray<TWeakObjectPtr<ARisingWall>> SpawnedWalls;

	// Called by your Pawn when input changes
	UFUNCTION(BlueprintCallable, Category="EarthWall|Input")
	void SetButtonsHeld(bool bLeftSide, bool bRightSide);
	
	ARisingWall* GetLookedAtWall(float MaxDist = 1200.f) const;
	void CleanupWalls();
	
private:
	USceneComponent* LeftHand = nullptr;
	USceneComponent* RightHand = nullptr;
	USceneComponent* ForwardComp = nullptr;

	bool bLS = false, bRS = false;

	EEarthWallState State = EEarthWallState::Idle;
	float CooldownRemaining = 0.f;

	bool AreAllButtonsHeld() const { return bLS && bRS; }

	bool TryGetGroundPoint(FVector& OutLoc, FRotator& OutRot) const;
	void SpawnWall(const FVector& GroundLoc, const FRotator& FacingRot);
		
};
