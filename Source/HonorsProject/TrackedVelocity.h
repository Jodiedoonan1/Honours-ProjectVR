// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TrackedVelocity.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HONORSPROJECT_API UTrackedVelocity : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTrackedVelocity();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tracked Velocity")
	FComponentReference TargetRef;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tracked Velocity")
	float Smoothing = 20.f;
	
	UFUNCTION(BlueprintCallable, Category="Tracked Velocity")
	FVector GetVelocity() const { return RawVelocity; }
	
	UPROPERTY()
	USceneComponent* Target = nullptr;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	
	bool bHasLast = false;
	FVector LastPos = FVector::ZeroVector;

	FVector RawVelocity = FVector::ZeroVector;
		
};
