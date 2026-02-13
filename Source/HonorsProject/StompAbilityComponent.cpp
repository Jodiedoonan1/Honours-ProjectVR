// Fill out your copyright notice in the Description page of Project Settings.


#include "StompAbilityComponent.h"
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


// Called every frame
void UStompAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
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

	switch (Data.State)
	{
	case EFootState::Grounded:
		if (AboveRest >= LiftThreshold)
		{
			Data.State = EFootState::Lifted;
		}
		break;

	case EFootState::Lifted:
		if (Data.VerticalSpeed < -10.f)
		{
			Data.State = EFootState::Falling;
		}
		break;

	case EFootState::Falling:
		{
			const bool bLanded = AboveRest <= LandTolerance;
			const bool bFast = Data.VerticalSpeed <= -MinDownSpeed;

			if (bLanded && bFast)
			{
				const float StompStrength = FMath::Abs(Data.VerticalSpeed);
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
				}
				else
				{
					if (GetGroundInFront(GroundLoc, GroundRot))
					{
						SpawnRock(GroundLoc, GroundRot, StompStrength);
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

	// Sphere trace is way more reliable than a thin line in VR
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

bool UStompAbilityComponent::GetGroundInFront(FVector& OutLoc, FRotator& OutRot) const
{
	AActor* Owner = GetOwner();
	if (!Owner || !GetWorld()) return false;

	const USceneComponent* Ref = ForwardReference ? ForwardReference : Owner->GetRootComponent();
	if (!Ref) return false;

	const FVector Origin = Ref->GetComponentLocation();
	const FRotator Rot = Ref->GetComponentRotation();
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

void UStompAbilityComponent::SpawnRock(const FVector& GroundLoc, const FRotator& Rot, float StompStrength) const
{
	if (!RockClass || !GetWorld()) return;

    FVector SpawnLoc = GroundLoc;
    SpawnLoc.Z -= SpawnDownOffset;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // Spawn as ARisingRock
    ARisingRock* Rock = GetWorld()->SpawnActor<ARisingRock>(RockClass, SpawnLoc, Rot, Params);
    if (!Rock) return;

    
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
}