// Fill out your copyright notice in the Description page of Project Settings.


#include "EarthWallAbilityComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/Engine.h"

UEarthWallAbilityComponent::UEarthWallAbilityComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;
}

void UEarthWallAbilityComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner) return;

	LeftHand  = Cast<USceneComponent>(LeftHandRef.GetComponent(Owner));
	RightHand = Cast<USceneComponent>(RightHandRef.GetComponent(Owner));
	ForwardComp = Cast<USceneComponent>(ForwardRef.GetComponent(Owner));
	
	UE_LOG(LogTemp, Warning, TEXT("EarthWall: LeftHandRef=%s  RightHandRef=%s  ForwardRef=%s"),
	*GetNameSafe(LeftHand),
	*GetNameSafe(RightHand),
	*GetNameSafe(ForwardComp));
}

void UEarthWallAbilityComponent::SetButtonsHeld(bool bLeftSide, bool bRightSide)
{
	bLS = bLeftSide; bRS = bRightSide;
	
	UE_LOG(LogTemp, Warning, TEXT("EarthWall: SetButtonsHeld called -> bLS=%d bRS=%d"), (int32)bLS, (int32)bRS);
}

void UEarthWallAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (State == EEarthWallState::CastedCooldown)
	{
		CooldownRemaining -= DeltaTime;
		if (CooldownRemaining <= 0.f)
			State = EEarthWallState::Idle;
		return;
	}

	if (!LeftHand || !RightHand) return;

	if (!AreAllButtonsHeld())
	{
		State = EEarthWallState::Idle;
		return;
	}
	
	const FVector LForward = LeftHand->GetForwardVector();
	const FVector RForward = RightHand->GetForwardVector();
	
	const float LDownDot = FVector::DotProduct(LForward, -FVector::UpVector);
	const float RDownDot = FVector::DotProduct(RForward, -FVector::UpVector);
	
	const float LUpDot = FVector::DotProduct(LForward, FVector::UpVector);
	const float RUpDot = FVector::DotProduct(RForward, FVector::UpVector);

	switch (State)
	{
	case EEarthWallState::Idle:
	case EEarthWallState::HoldingButtons:
		{
			CleanupWalls();
			
			if (LDownDot >= DownDotThreshold && RDownDot >= DownDotThreshold)
			{
				State = EEarthWallState::PrimedDown;
			}
			else if (LUpDot >= UpDotThreshold && RUpDot >= UpDotThreshold)
			{
				ARisingWall* Looked = GetLookedAtWall();
				if (Looked)
				{
					State = EEarthWallState::PrimedUp;
				}
				else
				{
					State = EEarthWallState::HoldingButtons;
				}
			}
			else
			{
				State = EEarthWallState::HoldingButtons;
			}
			break;
		}

	case EEarthWallState::PrimedDown:
		{
			if (LUpDot >= UpDotThreshold && RUpDot >= UpDotThreshold)
			{
				FVector GroundLoc;
				FRotator GroundRot;
				if (TryGetGroundPoint(GroundLoc, GroundRot))
				{
					SpawnWall(GroundLoc, GroundRot);
				}

				State = EEarthWallState::CastedCooldown;
				CooldownRemaining = Cooldown;
			}
			break;
		}

	case EEarthWallState::PrimedUp:
		{
			if (LDownDot >= DownDotThreshold && RDownDot >= DownDotThreshold)
			{
				CleanupWalls();

				ARisingWall* Looked = GetLookedAtWall();
				if (Looked)
				{
					Looked->StartLoweringAndDestroy();
					
					SpawnedWalls.RemoveAll([Looked](const TWeakObjectPtr<ARisingWall>& W)
					{
						return !W.IsValid() || W.Get() == Looked;
					});

					State = EEarthWallState::CastedCooldown;
					CooldownRemaining = Cooldown;
				}
				else
				{
					State = EEarthWallState::HoldingButtons;
				}
			}
			break;
		}

	default:
		break;
	}
	
	
}

bool UEarthWallAbilityComponent::TryGetGroundPoint(FVector& OutLoc, FRotator& OutRot) const
{
	AActor* Owner = GetOwner();
	if (!Owner || !GetWorld()) return false;

	const USceneComponent* Ref = ForwardComp ? ForwardComp : Owner->GetRootComponent();
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
		false, Ignore, EDrawDebugTrace::None,
		Hit, true
	);

	if (!bHit) return false;

	OutLoc = Hit.Location;
	OutRot = FRotator(0.f, Rot.Yaw, 0.f);
	return true;
}

void UEarthWallAbilityComponent::SpawnWall(const FVector& GroundLoc, const FRotator& FacingRot)
{
	if (!WallClass || !GetWorld()) return;

	FVector SpawnLoc = GroundLoc;
	SpawnLoc.Z -= SpawnDownOffset;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ARisingWall* Wall = GetWorld()->SpawnActor<ARisingWall>(WallClass, SpawnLoc, FacingRot, Params);
	if (!Wall) return;
	
	SpawnedWalls.Add(Wall);
	
	Wall->StartRising();
}

void UEarthWallAbilityComponent::CleanupWalls()
{
	SpawnedWalls.RemoveAll([](const TWeakObjectPtr<ARisingWall>& W)
	{
		return !W.IsValid();
	});
}

ARisingWall* UEarthWallAbilityComponent::GetLookedAtWall(float MaxDist) const
{
	AActor* Owner = GetOwner();
	if (!Owner || !GetWorld()) return nullptr;
	
	const USceneComponent* ViewRef = ForwardComp ? ForwardComp : Owner->GetRootComponent();
	if (!ViewRef) return nullptr;

	const FVector Start = ViewRef->GetComponentLocation();
	const FVector End   = Start + ViewRef->GetForwardVector() * MaxDist;

	FHitResult Hit;
	TArray<AActor*> Ignore;
	Ignore.Add(Owner);

	const bool bHit = UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		Start,
		End,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		Ignore,
		EDrawDebugTrace::ForDuration,
		Hit,
		true
	);

	if (!bHit) return nullptr;

	return Cast<ARisingWall>(Hit.GetActor());
}