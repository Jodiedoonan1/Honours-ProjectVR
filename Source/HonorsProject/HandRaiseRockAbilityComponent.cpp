// Fill out your copyright notice in the Description page of Project Settings.


#include "HandRaiseRockAbilityComponent.h"
#include "TaskManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

UHandRaiseRockAbilityComponent::UHandRaiseRockAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UHandRaiseRockAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
	
	AActor* Owner = GetOwner();
	if (!Owner) return;

	LeftHand  = Cast<USceneComponent>(LeftHandRef.GetComponent(Owner));
	RightHand = Cast<USceneComponent>(RightHandRef.GetComponent(Owner));
	Head      = Cast<USceneComponent>(HeadRef.GetComponent(Owner));

	auto InitHand = [](USceneComponent* HandComp, FHandRaiseData& Data)
	{
		if (!HandComp) return;
		Data.LastZ = HandComp->GetComponentLocation().Z;
		Data.bHasLast = true;
	};

	InitHand(LeftHand, LeftData);
	InitHand(RightHand, RightData);
}

void UHandRaiseRockAbilityComponent::SetTriggersHeld(bool bLeftTrigger, bool bRightTrigger, bool bLeftButton, bool bRightButton)
{
	bLT = bLeftTrigger;
	bRT = bRightTrigger;
	bLB = bLeftButton;
	bRB = bRightButton;
}

static float GetHeadZ(USceneComponent* Head, AActor* Owner, float Fallback)
{
    if (Head) return Head->GetComponentLocation().Z;
    if (Owner) return Owner->GetActorLocation().Z + 160.f;
    return Fallback;
}

void UHandRaiseRockAbilityComponent::UpdateHand(
    USceneComponent* HandComp,
    bool bTriggerHeld, bool bButtonHeld,
    FHandRaiseData& Data,
    float DeltaTime
)
{
    if (!HandComp) return;
	
	ATaskManager* TaskManager = Cast<ATaskManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATaskManager::StaticClass()));
	
    // If trigger not held reset this hand 
    if (!bTriggerHeld || !bButtonHeld)
    {
        Data.State = EHandRaiseState::Idle;
        Data.bLowReset = false;
        Data.bHasLast = false;
        return;
    }

    // Cooldown
    if (Data.State == EHandRaiseState::Cooldown)
    {
        Data.CooldownRemaining -= DeltaTime;
        if (Data.CooldownRemaining <= 0.f)
        {
            Data.State = EHandRaiseState::Idle;
            Data.bLowReset = true; 
        }
        return;
    }

    const float HandZ = HandComp->GetComponentLocation().Z;

    if (!Data.bHasLast)
    {
        Data.LastZ = HandZ;
        Data.bHasLast = true;
        return;
    }

    const float UpSpeed = (HandZ - Data.LastZ) / FMath::Max(DeltaTime, KINDA_SMALL_NUMBER);

    AActor* Owner = GetOwner();
    const float HeadZ = GetHeadZ(Head, Owner, HandZ + 160.f);

    const bool bHandIsLow = HandZ <= (HeadZ - ArmMaxHeightFromHead);

    switch (Data.State)
    {
    case EHandRaiseState::Idle:
    {
        if (Data.bLowReset)
        {
            if (bHandIsLow)
            {
                Data.bLowReset = false;
                Data.State = EHandRaiseState::Armed;
                Data.ArmedStartZ = HandZ;
            }
            break;
        }

        if (bHandIsLow)
        {
            Data.State = EHandRaiseState::Armed;
            Data.ArmedStartZ = HandZ;
        }
        break;
    }

    case EHandRaiseState::Armed:
    {
        // Keep lowest point while low
        if (bHandIsLow)
        {
            Data.ArmedStartZ = FMath::Min(Data.ArmedStartZ, HandZ);
        }

        const float DeltaZ = HandZ - Data.ArmedStartZ;

        // Trigger
        if (DeltaZ >= RaiseMinDeltaZ && UpSpeed >= RaiseMinUpSpeed)
        {
            FHitResult Hit;
        	const float Strength = ComputeRaiseStrength(UpSpeed);
        	
        	if (ARisingRock* Rock = FindLookedAtRock(Hit))
        	{
        		Rock->LaunchFromStomp(Strength, HeadZ);
        		
        		if (TaskManager->IsExpectingRaisedExistingRock())
        		{
        			TaskManager->NotifyRaisedExistingRock();
        		}
        	}
        	else
        	{
        		FVector GroundLoc;
        		FRotator GroundRot;
        		if (GetGroundInFront(GroundLoc, GroundRot))
        		{
        			SpawnRock(GroundLoc, GroundRot, Strength, HeadZ);
        		}
        	}

            Data.State = EHandRaiseState::Cooldown;
            Data.CooldownRemaining = Cooldown;
        }

        break;
    }

    default:
        break;
    }

    Data.LastZ = HandZ;
}

void UHandRaiseRockAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!GetWorld()) return;

	UpdateHand(LeftHand,  bLT, bLB, LeftData,  DeltaTime);
	UpdateHand(RightHand, bRT, bRB, RightData, DeltaTime);
}

float UHandRaiseRockAbilityComponent::ComputeRaiseStrength(float UpSpeed) const
{
	const float T = FMath::Clamp(UpSpeed / FMath::Max(UpSpeedForMaxStrength, 1.f), 0.f, 1.f);
	return FMath::Lerp(MinStrength, MaxStrength, T);
}

ARisingRock* UHandRaiseRockAbilityComponent::FindLookedAtRock(FHitResult& OutHit) const
{
	if (!GetWorld()) return nullptr;

	const USceneComponent* Ref = Head ? Head : (GetOwner() ? GetOwner()->GetRootComponent() : nullptr);
	if (!Ref) return nullptr;

	const FVector Start = Ref->GetComponentLocation();
	const FVector Dir = Ref->GetForwardVector().GetSafeNormal();
	const FVector End = Start + Dir * TargetRockRange;

	TArray<AActor*> Ignore;
	if (AActor* Owner = GetOwner()) Ignore.Add(Owner);

	const bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		Start,
		End,
		TargetRockSphereRadius,
		UEngineTypes::ConvertToTraceType(TargetTraceChannel),
		false,
		Ignore,
		EDrawDebugTrace::None,
		OutHit,
		true
	);

	if (!bHit) return nullptr;

	return Cast<ARisingRock>(OutHit.GetActor());
}

bool UHandRaiseRockAbilityComponent::GetGroundInFront(FVector& OutLoc, FRotator& OutRot) const
{
	AActor* Owner = GetOwner();
	if (!Owner || !GetWorld()) return false;

	// Use Head as reference
	const USceneComponent* Ref = Head ? Head : Owner->GetRootComponent();
	if (!Ref) return false;

	const FVector Origin  = Ref->GetComponentLocation();
	const FRotator Rot    = Ref->GetComponentRotation();
	const FVector Forward = Rot.Vector();

	const FVector TargetXY = Origin + Forward * SpawnForwardDistance;

	const FVector Start = TargetXY + FVector(0,0,TraceUp);
	const FVector End   = TargetXY - FVector(0,0,TraceDown);

	FHitResult Hit;
	TArray<AActor*> Ignore;
	Ignore.Add(Owner);

	const bool bHit = UKismetSystemLibrary::LineTraceSingle(
		GetWorld(), Start, End,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false, Ignore,
		EDrawDebugTrace::None,
		Hit, true
	);

	if (!bHit) return false;

	OutLoc = Hit.Location;
	OutRot = FRotator(0.f, Rot.Yaw, 0.f);
	return true;
}

ARisingRock* UHandRaiseRockAbilityComponent::SpawnRock(
	const FVector& GroundLoc,
	const FRotator& Rot,
	float Strength,
	float HeadZ
) const
{
	if (!RockClass || !GetWorld()) return nullptr;

	FVector SpawnLoc = GroundLoc;
	SpawnLoc.Z -= SpawnDownOffset;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ARisingRock* Rock = GetWorld()->SpawnActor<ARisingRock>(RockClass, SpawnLoc, Rot, Params);
	if (!Rock) return nullptr;
	
	ATaskManager* TaskManager = Cast<ATaskManager>(
	UGameplayStatics::GetActorOfClass(GetWorld(), ATaskManager::StaticClass()));

	if (Rock && TaskManager->ShouldUseRockColourFeedback())
	{
		const bool bWasCorrect = TaskManager->IsExpectingHandRock();

		Rock->ApplyTaskFeedback(bWasCorrect);

		if (bWasCorrect)
		{
			TaskManager->NotifyHandRockCreated();
		}
	}

	Rock->LaunchFromStomp(Strength, HeadZ);
	return Rock;
}