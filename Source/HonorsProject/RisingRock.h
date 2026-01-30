// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RisingRock.generated.h"

UCLASS()
class HONORSPROJECT_API ARisingRock : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARisingRock();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
	void OnMeshBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Punch")
	float PunchImpulseStrength = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Punch")
	float PunchCooldown = 0.15f;

	bool bCanBePunched = true;
	FTimerHandle PunchCooldownTimer;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* Mesh = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rise")
	float RiseHeight = 80.f;

	// How long it takes to rise
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rise")
	float RiseTime = 0.35f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rise")
	bool bEnableCollision = true;
	
private:
	FVector StartLoc;
	FVector EndLoc;
	float Elapsed = 0.f;

	void SetCollisionEnabled(bool bEnabled);
};
