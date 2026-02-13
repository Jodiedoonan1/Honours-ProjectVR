// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LaunchPillar.generated.h"

UCLASS()
class HONORSPROJECT_API ALaunchPillar : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALaunchPillar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable, Category="LaunchPillar")
	void InitLaunch(const FVector& InLaunchDir);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* Mesh = nullptr;

	// How far the pillar rises along LaunchDir (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LaunchPillar|Move")
	float RiseDistance = 180.f;

	// Time to rise (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LaunchPillar|Move")
	float RiseTime = 0.25f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LaunchPillar|Collision")
	bool bTraceableWhileMoving = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LaunchPillar|Collision")
	bool bEnableCollisionWhenFinished = true;

	// Optional cleanup
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LaunchPillar|Lifetime")
	bool bAutoDestroy = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LaunchPillar|Lifetime", meta=(EditCondition="bAutoDestroy"))
	float LifeAfterRise = 2.0f;
	
private:
	FVector LaunchDir = FVector::UpVector;

	bool bMoving = false;
	float Elapsed = 0.f;
	FVector MoveStart = FVector::ZeroVector;
	FVector MoveEnd   = FVector::ZeroVector;

	void BeginRise();
	void SetCollisionEnabled(bool bEnabled);

};
