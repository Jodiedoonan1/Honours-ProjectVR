// Fill out your copyright notice in the Description page of Project Settings.


#include "StompAbilityComponent.h"

#include "NetworkReplayStreaming.h"
#include "TaskManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
UStompAbilityComponent::UStompAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UStompAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
	AActor* Owner = GetOwner();
	if (!Owner) return;

	FootLeft  = Cast<USceneComponent>(FootLeftRef.GetComponent(Owner));
	FootRight = Cast<USceneComponent>(FootRightRef.GetComponent(Owner));
	ForwardReference = Cast<USceneComponent>(ForwardRef.GetComponent(Owner));
	HeadComponent = Cast<USceneComponent>(HeadRef.GetComponent(Owner));
	Calibrate();
	
}

static const TCHAR* FootSideName(const FFootStompData& Data, const FFootStompData& LeftData)
{
	return (&Data == &LeftData) ? TEXT("Left") : TEXT("Right");
}


// Called every frame
void UStompAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (StompLockedRemaining > 0.0f)
	{
		StompLockedRemaining = StompLockedRemaining - DeltaTime;
	}
	UpdateFoot(FootLeft, Left, DeltaTime);
	UpdateFoot(FootRight, Right, DeltaTime);
}

void UStompAbilityComponent::Calibrate()
{
	if (FootLeft)
	{
		const float Z = FootLeft->GetComponentLocation().Z;
		Left.RestZ = Z;
		Left.LastZ = Z;
		Left.State = EFootState::Grounded;
		Left.CooldownRemaining = 0.f;
	}
	if (FootRight)
	{
		const float Z = FootRight->GetComponentLocation().Z;
		Right.RestZ = Z;
		Right.LastZ = Z;
		Right.State = EFootState::Grounded;
		Right.CooldownRemaining = 0.f;
	}
}

void UStompAbilityComponent::UpdateFoot(USceneComponent* Foot, FFootStompData& Data, float DeltaTime)
{
	if (!Foot || !GetWorld()) return;
	
	if (StompLockedRemaining > 0.0f)
	{
		Data.LastZ = Foot->GetComponentLocation().Z;
		return;
	}
	
	ATaskManager* TaskManager = Cast<ATaskManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATaskManager::StaticClass()));

	if (Data.State == EFootState::Cooldown)
	{
		Data.CooldownRemaining -= DeltaTime;
		if (Data.CooldownRemaining <= 0.0f)
		{
			Data.State = EFootState::Grounded;
		}
	}

	const float CurrentZ = Foot->GetComponentLocation().Z;
	Data.VerticalSpeed = (CurrentZ - Data.LastZ) / FMath::Max(DeltaTime, KINDA_SMALL_NUMBER);

	const float AboveRest = CurrentZ - Data.RestZ;
	
	static float DebugAccumulator = 0.f;
	DebugAccumulator += DeltaTime;

	if (DebugAccumulator >= 0.10f)
	{
		DebugAccumulator = 0.f;

		const TCHAR* Side = (Foot == FootLeft) ? TEXT("Left") : TEXT("Right");

		const TCHAR* StateStr =
			(Data.State == EFootState::Grounded) ? TEXT("Grounded") :
			(Data.State == EFootState::Lifted)   ? TEXT("Lifted")   :
			(Data.State == EFootState::Falling)  ? TEXT("Falling")  :
			(Data.State == EFootState::Cooldown) ? TEXT("Cooldown") : TEXT("Unknown");
		
	}
	
	switch (Data.State)
	{
	case EFootState::Grounded:
		if (AboveRest >= LiftThreshold)
		{
			Data.State = EFootState::Lifted;
			Data.MaxAboveRest = AboveRest;
		}
		break;

	case EFootState::Lifted:
		Data.MaxAboveRest = FMath::Max(Data.MaxAboveRest, AboveRest);
		if (Data.VerticalSpeed < -10.f)
		{
			Data.State = EFootState::Falling;
			
			Data.PeakDownSpeedNearGround = 0.0f;
			Data.bEnteredNearGroundWindow = false;
		}
		break;

	case EFootState::Falling:
		{
			Data.MaxAboveRest = FMath::Max(Data.MaxAboveRest, AboveRest);
			const bool bLanded = AboveRest <= LandTolerance;
			const bool bFastNearGround = Data.bEnteredNearGroundWindow && (Data.PeakDownSpeedNearGround <= -MinDownSpeed);
			
			if (AboveRest <= NearGroundWindow)
			{
				Data.bEnteredNearGroundWindow = true;
				
				if (Data.PeakDownSpeedNearGround == 0.0f)
				{
					Data.PeakDownSpeedNearGround = Data.VerticalSpeed;
				}
				else
				{
					Data.PeakDownSpeedNearGround = FMath::Min(Data.PeakDownSpeedNearGround, Data.VerticalSpeed);
				}
			}

			if (bLanded && bFastNearGround)
			{
				const float UsedSpeed = Data.PeakDownSpeedNearGround;
				const float StompStrength = FMath::Abs(UsedSpeed);
				const float LiftT  = FMath::Clamp(Data.MaxAboveRest / FMath::Max(MaxLiftForFullSize, 1.f), 0.f, 1.f);
				const float SpeedT = FMath::Clamp(
			(StompStrength - MinDownSpeed) / FMath::Max(MaxDownSpeedForFullSize - MinDownSpeed, 1.f),
			0.0f, 1.0f);
				const float SizeT = FMath::Clamp(LiftT * LiftWeight + SpeedT * SpeedWeight, 0.f, 1.f);
				const float ShapedT = FMath::Pow(SizeT, SizeCurvePower);
				const float RockScale = FMath::Lerp(MinRockScale, MaxRockScale, ShapedT);
				
				const TCHAR* Side = (Foot == FootLeft) ? TEXT("Left") : TEXT("Right");

				UE_LOG(LogTemp, Warning,
					TEXT("[Stomp TRIGGER][%s] Above=%.1f MaxAbove=%.1f  VSpeed=%.1f VPeakNear=%.1f Strength=%.1f  bLanded=%d bFast=%d | LiftT=%.2f SpeedT=%.2f SizeT=%.2f ShapedT=%.2f RockScale=%.2f | MinDown=%.1f MaxDownFull=%.1f MaxLiftFull=%.1f"),
					Side,
					AboveRest,
					Data.MaxAboveRest,
					Data.VerticalSpeed,
					Data.PeakDownSpeedNearGround,
					StompStrength,
					bLanded ? 1 : 0,
					bFastNearGround ? 1 : 0,
					LiftT,
					SpeedT,
					SizeT,
					ShapedT,
					RockScale,
					MinDownSpeed,
					MaxDownSpeedForFullSize,
					MaxLiftForFullSize
				);
				
				FVector GroundLoc;
				FRotator GroundRot;
				FHitResult LookHit;
				if (ARisingRock* ExistingRock = FindLookedAtRock(LookHit))
				{
					AActor* Owner = GetOwner();
					float HeadZ = 0.f;
					if (HeadComponent) HeadZ = HeadComponent->GetComponentLocation().Z;
					else if (Owner)   HeadZ = Owner->GetActorLocation().Z + 160.f;
					else              HeadZ = LookHit.Location.Z + 160.f;

					ExistingRock->LaunchFromStomp(StompStrength, HeadZ);
					
					if (TaskManager->IsExpectingRaisedExistingRock())
					{
						TaskManager->NotifyRaisedExistingRock();
					}
				}
				else
				{
					if (GetGroundInFront(GroundLoc, GroundRot, RockScale))
					{
						ARisingRock* NewRock = SpawnRock(GroundLoc, GroundRot, StompStrength, RockScale);

						if (NewRock)
						{
							const ERockSize RockSize = GetRockSizeFromScale(RockScale);

							//ATaskManager* TaskManager = Cast<ATaskManager>(
							//	UGameplayStatics::GetActorOfClass(GetWorld(), ATaskManager::StaticClass()));

							if (TaskManager->ShouldUseRockColourFeedback())
							{
								bool bWasCorrect = false;
								
								const ERockSize TargetSize = TaskManager->GetCurrentTargetRockSize();
								
								if (TaskManager->IsExpectingStompRock())
								{
									bWasCorrect = (RockSize == TargetSize);
								}
								else
								{
									bWasCorrect = false;
								}

								NewRock->ApplyTaskFeedback(bWasCorrect);

								if (bWasCorrect)
								{
									TaskManager->NotifyRockCreated(RockSize);
								}

								UE_LOG(LogTemp, Warning, TEXT("Rock created. Scale = %.2f, Size = %s, Target = %s, Correct = %s"),
									RockScale,
									*UEnum::GetValueAsString(RockSize),
									*UEnum::GetValueAsString(TargetSize),
									bWasCorrect ? TEXT("Yes") : TEXT("No"));
							}
						}
					}
				}

				Data.State = EFootState::Cooldown;
				Data.CooldownRemaining = Cooldown;
			}
			else if (bLanded)
			{
				Data.State = EFootState::Grounded;
			}
			break;
		}

	default:
		break;
	}

	Data.LastZ = CurrentZ;
}

ARisingRock* UStompAbilityComponent::FindLookedAtRock(FHitResult& OutHit) const
{
	if (!GetWorld()) return nullptr;

	const USceneComponent* Head = HeadComponent ? HeadComponent : ForwardReference;
	if (!Head) return nullptr;

	const FVector Start = Head->GetComponentLocation();
	const FVector Dir   = Head->GetForwardVector().GetSafeNormal();
	const FVector End   = Start + Dir * TargetRockRange;

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
	
	ARisingRock* Rock = Cast<ARisingRock>(OutHit.GetActor());
	return Rock;
}

bool UStompAbilityComponent::GetGroundInFront(FVector& OutLoc, FRotator& OutRot, float RockScale) const
{
	AActor* Owner = GetOwner();
	if (!Owner || !GetWorld()) return false;

	const USceneComponent* Ref = ForwardReference ? ForwardReference : Owner->GetRootComponent();
	if (!Ref) return false;

	const FVector Origin = Ref->GetComponentLocation();
	const FRotator Rot = Ref->GetComponentRotation();
	const FVector Forward = Rot.Vector();

	const float Range = FMath::Max(MaxRockScale - MinRockScale, KINDA_SMALL_NUMBER);
	const float NormalizedScale = FMath::Clamp((RockScale - MinRockScale) / Range, 0.0f, 1.0f);

	// Small rocks closer, large rocks further
	const float ActualSpawnDistance = FMath::Lerp(SmallRockSpawnDistance, LargeRockSpawnDistance, NormalizedScale);

	const FVector TargetXY = Origin + Forward * ActualSpawnDistance;

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

void UStompAbilityComponent::CantStomp()
{
	StompLockedRemaining = StompLockedDuration;
}

ARisingRock* UStompAbilityComponent::SpawnRock(const FVector& GroundLoc, const FRotator& Rot, float StompStrength, float RockScale) const
{
	if (!RockClass || !GetWorld()) return nullptr;

    FVector SpawnLoc = GroundLoc;
    SpawnLoc.Z -= SpawnDownOffset;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
    ARisingRock* Rock = GetWorld()->SpawnActor<ARisingRock>(RockClass, SpawnLoc, Rot, Params);
    if (!Rock) return nullptr;
	
	Rock->SetActorScale3D(FVector(RockScale));
	
	Rock->SetSizeScale(RockScale);
    
    AActor* Owner = GetOwner();
    float HeadZ = 0.f;

    if (HeadComponent)
    {
        HeadZ = HeadComponent->GetComponentLocation().Z;
    }
    else if (Owner)
    {
        HeadZ = Owner->GetActorLocation().Z + 160.f; // fallback
    }
    else
    {
        HeadZ = SpawnLoc.Z + 160.f;
    }
	
    Rock->LaunchFromStomp(StompStrength, HeadZ);
	
	return Rock;
}

ERockSize UStompAbilityComponent::GetRockSizeFromScale(float RockScale) const
{
	const float Range = FMath::Max(MaxRockScale - MinRockScale, KINDA_SMALL_NUMBER);
	const float Normalized = FMath::Clamp((RockScale - MinRockScale) / Range, 0.0f, 1.0f);

	if (Normalized <= 0.33f)
	{
		return ERockSize::Small;
	}
	else if (Normalized <= 0.66f)
	{
		return ERockSize::Medium;
	}

	return ERockSize::Large;
}