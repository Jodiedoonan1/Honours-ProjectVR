// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RisingWall.generated.h"

UCLASS()
class HONORSPROJECT_API ARisingWall : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARisingWall();

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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* Mesh = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rise")
	float RiseHeight = 140.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rise")
	float RiseTime = 0.35f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rise")
	bool bEnableCollisionWhenFinished = true;
	
	UFUNCTION(BlueprintCallable, Category="Wall|Move")
	void StartRising();
	
	UFUNCTION(BlueprintCallable, Category="Wall|Move")
	void StartLoweringAndDestroy();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Punch|Physics")
	float ArmEffectiveMassKg = 3.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Punch|Tuning")
	float PunchMinSpeed = 60.f;      

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Punch|Tuning")
	float PunchMaxSpeed = 1900.f;     

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Punch|Tuning")
	float MinDeltaV = 30.f;          

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Punch|Tuning")
	float MaxDeltaV = 2000.f;        

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Punch|Tuning")
	float PunchPower = 2.2f;         

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Punch|Tuning")
	float MomentumTransfer = 0.8f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Punch")
	float PunchCooldown = 0.15f;
	
	bool bCanBePunched = true;
	FTimerHandle PunchCooldownTimer;

private:
	FVector MoveStart;
	FVector MoveEnd;
	float Elapsed = 0.0f;
	
	bool bMoving = false;
	bool bLowering = false;
	bool bDestroyWhenDone = false;

	void BeginMoveTo(const FVector& Target, bool bDestroyAfter);
	void SetCollisionEnabled(bool bEnabled);

};
