// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RockTypes.h"
#include "Materials/MaterialInstanceDynamic.h"
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
	
	UFUNCTION()
	void OnMeshHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);
	
	UFUNCTION()
	void ApplyTaskFeedback(bool bWasCorrect);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Punch|Physics")
	float ArmEffectiveMassKg = 3.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Punch|Tuning")
	float PunchMinSpeed = 60.f;      

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Punch|Tuning")
	float PunchMaxSpeed = 500.f;     

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Punch|Tuning")
	float MinDeltaV = 30.f;          

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Punch|Tuning")
	float MaxDeltaV = 2000.f;        

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Punch|Tuning")
	float PunchPower = 2.2f;         

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Punch|Tuning")
	float MomentumTransfer = 0.8f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Punch")
	float PunchImpulseStrength = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Punch")
	float PunchCooldown = 0.15f;

	bool bCanBePunched = true;
	FTimerHandle PunchCooldownTimer;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* Mesh = nullptr;
	
	UPROPERTY(EditAnywhere, Category="Break")
	TSubclassOf<AActor> BrokenRockClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Break")
	bool bCanBreak = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Break")
	float BreakImpactSpeed = 300.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp")
	float MinUpImpulse = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp")
	float MaxUpImpulse = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp")
	float StrengthToImpulse = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stomp")
	float MaxHeight = 10.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rise")
	bool bEnableCollision = true;
	
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicRockMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "Task Feedback")
	FName TintParameterName = TEXT("TintColour");

	UPROPERTY(EditAnywhere, Category = "Task Feedback")
	FLinearColor CorrectFeedbackColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.6f);

	UPROPERTY(EditAnywhere, Category = "Task Feedback")
	FLinearColor WrongFeedbackColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.6f);
	
	bool bPendingTaskFeedbackCorrect = false;

	UPROPERTY(EditAnywhere, Category = "Task Feedback")
	float FeedbackBreakDelay = 1.0f;

	FTimerHandle FeedbackBreakTimer;
	FTimerHandle FeedbackColourTimer;

	bool bIsPhysicsActive = false;
	
	float BaseMaxHeight = 0.f;
	
	FTimerHandle CCDEnableTimer;

	UFUNCTION(BlueprintCallable)
	void SetSizeScale(float Scale);

	void LaunchFromStomp(float StompStrength, float MaxAllowedWorldZ);
	
	UFUNCTION()
	void EnableCCD();
	
	UFUNCTION()
	void RockBreak();
	
	void ChangeRockColour();
	
private:
	bool bHasLaunched = false;
	float MaxZ = 0.0f;
};
