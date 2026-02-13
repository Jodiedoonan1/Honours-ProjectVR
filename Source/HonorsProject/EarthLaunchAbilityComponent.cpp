// Fill out your copyright notice in the Description page of Project Settings.


#include "EarthLaunchAbilityComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"

UEarthLaunchAbilityComponent::UEarthLaunchAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEarthLaunchAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
	
	AActor* Owner = GetOwner();
	if (!Owner) return;

	LeftHand  = Cast<USceneComponent>(LeftHandRef.GetComponent(Owner));
	RightHand = Cast<USceneComponent>(RightHandRef.GetComponent(Owner));
	ForwardComp = Cast<USceneComponent>(ForwardRef.GetComponent(Owner));
    
    LeftVel  = Cast<UTrackedVelocity>(LeftVelocityRef.GetComponent(Owner));
    RightVel = Cast<UTrackedVelocity>(RightVelocityRef.GetComponent(Owner));
}

void UEarthLaunchAbilityComponent::SetTriggersHeld(bool bLeftTrigger, bool bRightTrigger)
{
	bLT = bLeftTrigger; bRT = bRightTrigger;
    
    UE_LOG(LogTemp, Warning, TEXT("EarthLaunch: SetTriggerssHeld called -> bLT=%d bRT=%d"), (int32)bLT, (int32)bRT);
}

void UEarthLaunchAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (State == EEarthLaunchState::Cooldown)
    {
        CooldownRemaining -= DeltaTime;
        if (CooldownRemaining <= 0.f)
        {
            State = EEarthLaunchState::Idle;
        }
        return;
    }
    
    if (bLaunching)
    {
        AActor* Owner = GetOwner();
        if (!Owner) return;

        // gravity
        LaunchVelocity.Z -= LaunchGravity * DeltaTime;

        // optional drag
        if (LaunchDrag > 0.f)
        {
            LaunchVelocity -= LaunchVelocity * LaunchDrag * DeltaTime;
        }

        FVector Delta = LaunchVelocity * DeltaTime;

        FHitResult Hit;
       // Owner->AddActorWorldOffset(Delta, true, &Hit, ETeleportType::None);
        
        UCapsuleComponent* Capsule = Owner->FindComponentByClass<UCapsuleComponent>();
        USceneComponent* MoveComp = Capsule ? (USceneComponent*)Capsule : Owner->GetRootComponent();

        MoveComp->AddWorldOffset(Delta, true, &Hit, ETeleportType::None);

        if (Hit.IsValidBlockingHit())
        {
            // simple stop on ground/walls
            LaunchVelocity = FVector::ZeroVector;
            bLaunching = false;
            
            UE_LOG(LogTemp, Warning, TEXT("Launch BLOCKED by %s comp %s normal=%s loc=%s"),
        Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("None"),
        Hit.GetComponent() ? *Hit.GetComponent()->GetName() : TEXT("None"),
        *Hit.Normal.ToString(),
        *Hit.ImpactPoint.ToString()
    );
        }

        // you can also stop when velocity is small
        if (LaunchVelocity.SizeSquared() < 25.f)
        {
            LaunchVelocity = FVector::ZeroVector;
            bLaunching = false;
        }
    }

    if (!LeftHand || !RightHand) return;

    // If triggers released, reset
    if (!AreAllButtonsHeld())
    {
        State = EEarthLaunchState::Idle;
        PrimeRemaining = 0.f;
        return;
    }

    // Reference forward 
    const USceneComponent* Ref = ForwardComp ? ForwardComp : GetOwner()->GetRootComponent();
    if (!Ref) return;

    const FVector Fwd = Ref->GetForwardVector().GetSafeNormal();
    const FVector Up = FVector::UpVector;

    // Hand directions
    const FVector LDir = LeftHand->GetForwardVector().GetSafeNormal();
    const FVector RDir = RightHand->GetForwardVector().GetSafeNormal();

    // Pose metrics
    const float LDownDot = FVector::DotProduct(LDir, -Up);
    const float RDownDot = FVector::DotProduct(RDir, -Up);

    const float LBackDot = FVector::DotProduct(LDir, -Fwd);
    const float RBackDot = FVector::DotProduct(RDir, -Fwd);

    const float LFwdDot = FVector::DotProduct(LDir, Fwd);
    const float RFwdDot = FVector::DotProduct(RDir, Fwd);

    const float LUpDot = FVector::DotProduct(LDir, Up);
    const float RUpDot = FVector::DotProduct(RDir, Up);
    
    if (State == EEarthLaunchState::HoldingButtons || State == EEarthLaunchState::Idle)
    {
        UE_LOG(LogTemp, Verbose, TEXT("EarthLaunch: State=%d LDown=%.2f RDown=%.2f LBack=%.2f RBack=%.2f LFwd=%.2f RFwd=%.2f LUp=%.2f RUp=%.2f"),
            (int32)State, LDownDot, RDownDot, LBackDot, RBackDot, LFwdDot, RFwdDot, LUpDot, RUpDot);
    }

    // Prime timeout
    if (State == EEarthLaunchState::PrimedDownBack)
    {
        PrimeRemaining -= DeltaTime;
        if (PrimeRemaining <= 0.f)
        {
            State = EEarthLaunchState::HoldingButtons;
        }
    }

    switch (State)
    {
    case EEarthLaunchState::Idle:
    case EEarthLaunchState::HoldingButtons:
    {
        // Prime when both hands are behind + pointing down to floor
        const bool bDownOK = (LDownDot >= DownDotThreshold) && (RDownDot >= DownDotThreshold);
        const bool bBackOK = (LBackDot >= BackDotThreshold) && (RBackDot >= BackDotThreshold);

        if (bDownOK && bBackOK)
        {
            State = EEarthLaunchState::PrimedDownBack;
            PrimeRemaining = PrimeTimeout;
        }
        else
        {
            State = EEarthLaunchState::HoldingButtons;
        }
        break;
    }

    case EEarthLaunchState::PrimedDownBack:
    {
        // Cast when both hands move forward and are angled upward
        const bool bForwardOK = (LFwdDot >= ForwardDotThreshold) && (RFwdDot >= ForwardDotThreshold);

        const float AvgUpDot = 0.5f * (LUpDot + RUpDot);
        const bool bUpAngleOK = (AvgUpDot >= LaunchUpMin) && (AvgUpDot <= LaunchUpMax);

        if (bForwardOK && bUpAngleOK)
        {
            FVector GroundLoc;
            FRotator GroundRot;

            if (TryGetGroundPoint(GroundLoc, GroundRot))
            {
                const FVector LaunchDir = ComputeLaunchDirection(AvgUpDot);

                // Spawn angled pillar
                SpawnLaunchPillar(GroundLoc, GroundRot, LaunchDir);

                // Launch the player
                float LaunchSpeed = ComputeLaunchSpeedFromHands(LaunchDir);
                ApplyLaunch(LaunchDir, LaunchSpeed);

                State = EEarthLaunchState::Cooldown;
                CooldownRemaining = Cooldown;
            }
            else
            {
                // Couldn't find ground — stay holding
                State = EEarthLaunchState::HoldingButtons;
            }
        }
        break;
    }

    default:
        break;
    }
}

bool UEarthLaunchAbilityComponent::TryGetGroundPoint(FVector& OutLoc, FRotator& OutRot) const
{
    AActor* Owner = GetOwner();
    if (!Owner || !GetWorld()) return false;

    const USceneComponent* Ref = ForwardComp ? ForwardComp : Owner->GetRootComponent();
    if (!Ref) return false;

    const FVector Origin = Ref->GetComponentLocation();
    const FVector Forward = Ref->GetForwardVector().GetSafeNormal();

    // Point slightly in front of player
    const FVector TargetXY = Origin - Forward * SpawnBackwardDistance;

    const FVector Start = TargetXY + FVector(0, 0, TraceUp);
    const FVector End   = TargetXY - FVector(0, 0, TraceDown);

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
        EDrawDebugTrace::None,
        Hit,
        true
    );

    if (!bHit) return false;

    OutLoc = Hit.Location;

    // Face the same yaw as the HMD forward
    const FRotator YawRot(0.f, Ref->GetComponentRotation().Yaw, 0.f);
    OutRot = YawRot;

    return true;
}

FVector UEarthLaunchAbilityComponent::ComputeLaunchDirection(float AvgUpDot) const
{
    AActor* Owner = GetOwner();
    const USceneComponent* Ref = (Owner && ForwardComp) ? ForwardComp : (Owner ? Owner->GetRootComponent() : nullptr);
    if (!Ref) return FVector::UpVector;

    const FVector Fwd = Ref->GetForwardVector().GetSafeNormal();
    const FVector Up = FVector::UpVector;

    // 0..1 within the allowed up-dot band
    float UpAmount = (AvgUpDot - LaunchUpMin) / FMath::Max(LaunchUpMax - LaunchUpMin, 0.01f);
    UpAmount = FMath::Clamp(UpAmount, 0.f, 1.f);

    // More up-dot = more vertical component
    const float UpScale = FMath::Lerp(LaunchUpScaleMin, LaunchUpScaleMax, UpAmount);

    return (Fwd + Up * UpScale).GetSafeNormal();
}

float UEarthLaunchAbilityComponent::ComputeLaunchSpeedFromHands(const FVector& LaunchDir) const
{
    if (!LeftVel || !RightVel) return MaxLaunchSpeed; // fallback

    const FVector L = LeftVel->GetVelocity();
    const FVector R = RightVel->GetVelocity();

    const FVector Avg = 0.5f * (L + R);

    // Only count motion in the launch direction (ignores sideways noise)
    const float SpeedAlong = FMath::Max(0.f, FVector::DotProduct(Avg, LaunchDir)); // cm/s

    // Map 0..MaxHandSpeedForFullLaunch -> 0..1
    const float T = FMath::Clamp(SpeedAlong / FMath::Max(MaxHandSpeedForFullLaunch, 1.f), 0.f, 1.f);

    return FMath::Lerp(MinLaunchSpeed, MaxLaunchSpeed, T); // cm/s launch speed
}

void UEarthLaunchAbilityComponent::ApplyLaunch(const FVector& LaunchDir, float LaunchSpeed)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    LaunchVelocity = LaunchDir.GetSafeNormal() * LaunchSpeed;
    bLaunching = true;

    UE_LOG(LogTemp, Warning, TEXT("EarthLaunch: Could not apply launch (no sim physics root, no FloatingPawnMovement). Hook ApplyLaunch to your movement system."));
}

void UEarthLaunchAbilityComponent::SpawnLaunchPillar(const FVector& GroundLoc, const FRotator& FacingRot, const FVector& LaunchDir)
{
    if (!PillarClass || !GetWorld()) return;

    FVector SpawnLoc = GroundLoc;
    SpawnLoc.Z -= SpawnDownOffset;

    // Rotate pillar to match launch direction
    FRotator DirRot = LaunchDir.Rotation();
    DirRot.Yaw += 180.f;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ALaunchPillar* Pillar = GetWorld()->SpawnActor<ALaunchPillar>(PillarClass, SpawnLoc, DirRot, Params);
    if (!Pillar) return;
    
    Pillar->InitLaunch(LaunchDir);
}