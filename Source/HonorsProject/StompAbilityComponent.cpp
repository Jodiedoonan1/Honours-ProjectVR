// Fill out your copyright notice in the Description page of Project Settings.


#include "StompAbilityComponent.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
UStompAbilityComponent::UStompAbilityComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
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
	Calibrate();
	// ...
	
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
		if (Data.CooldownRemaining <= 0.f)
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
				FVector GroundLoc;
				FRotator GroundRot;
				if (GetGroundInFront(GroundLoc, GroundRot))
				{
					SpawnRock(GroundLoc, GroundRot);
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

void UStompAbilityComponent::SpawnRock(const FVector& GroundLoc, const FRotator& Rot) const
{
	if (!RockClass || !GetWorld()) return;

	FVector SpawnLoc = GroundLoc;
	SpawnLoc.Z -= SpawnDownOffset;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GetWorld()->SpawnActor<AActor>(RockClass, SpawnLoc, Rot, Params);
}