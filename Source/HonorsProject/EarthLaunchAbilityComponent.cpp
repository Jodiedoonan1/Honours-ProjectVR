// Fill out your copyright notice in the Description page of Project Settings.


#include "EarthLaunchAbilityComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"

static float GLaunchDebugAccum = 0.f;

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
    
    WaistComp     = Cast<USceneComponent>(WaistRef.GetComponent(Owner));
    LeftFootComp  = Cast<USceneComponent>(LeftFootRef.GetComponent(Owner));
    RightFootComp = Cast<USceneComponent>(RightFootRef.GetComponent(Owner));
    
    LeftVel  = Cast<UTrackedVelocity>(LeftVelocityRef.GetComponent(Owner));
    RightVel = Cast<UTrackedVelocity>(RightVelocityRef.GetComponent(Owner));
    
    if (WaistComp && LeftFootComp && RightFootComp)
    {
        const float FeetZ = 0.5f * (LeftFootComp->GetComponentLocation().Z + RightFootComp->GetComponentLocation().Z);
        const float WaistZ = WaistComp->GetComponentLocation().Z;
        StandCompression = WaistZ - FeetZ;
    }
}

void UEarthLaunchAbilityComponent::SetTriggersHeld(bool bLeftTrigger, bool bRightTrigger)
{
	bLT = bLeftTrigger; bRT = bRightTrigger;
}

void UEarthLaunchAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    AActor* Owner = GetOwner();
    UCapsuleComponent* Capsule = Owner->FindComponentByClass<UCapsuleComponent>();
    
    const FVector CapLoc = Capsule->GetComponentLocation();
    const float CapRadius = Capsule->GetScaledCapsuleRadius();
    const float CapHalf = Capsule->GetScaledCapsuleHalfHeight();
    const FQuat CapQuat = Capsule->GetComponentQuat();

    // Draw capsule (green)
   /* DrawDebugCapsule(
        GetWorld(),
        CapLoc,
        CapHalf,
        CapRadius,
        CapQuat,
        FColor::Green,
        false,
        0.0f,   
        0,
        1.5f
    ); */
	
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
       // AActor* Owner = GetOwner();
        if (!Owner) return;

        float GravityThisFrame = (LaunchVelocity.Z > 0.f) ? 1400.f : 2200.f;
        if (LaunchVelocity.Z < 0.f)
        {
            const float FastFallT = GetFastFallT();
            GravityThisFrame = FMath::Lerp(GravityThisFrame, FastFallGravity, FastFallT);
        }

        LaunchVelocity.Z -= GravityThisFrame * DeltaTime;
        
        if (LaunchDrag > 0.f)
        {
            LaunchVelocity -= LaunchVelocity * LaunchDrag * DeltaTime;
        }
        
        const USceneComponent* Ref = ForwardComp ? ForwardComp : Owner->GetRootComponent();
        if (Ref)
        {
            const FVector DesiredFlat = FVector(Ref->GetForwardVector().X, Ref->GetForwardVector().Y, 0.f).GetSafeNormal();

            if (!DesiredFlat.IsNearlyZero())
            {
                FVector CurrentFlat = FVector(LaunchVelocity.X, LaunchVelocity.Y, 0.f);
                const float FlatSpeed = CurrentFlat.Size();

                FVector TargetFlat = DesiredFlat * FlatSpeed;
                CurrentFlat = FMath::VInterpTo(CurrentFlat, TargetFlat, DeltaTime, 3.0f);

                LaunchVelocity.X = CurrentFlat.X;
                LaunchVelocity.Y = CurrentFlat.Y;
            }
        }

        FVector Delta = LaunchVelocity * DeltaTime;

        FHitResult Hit;
        
       // UCapsuleComponent* Capsule = Owner->FindComponentByClass<UCapsuleComponent>();
       // USceneComponent* MoveComp = Capsule ? (USceneComponent*)Capsule : Owner->GetRootComponent();
        
       // UCapsuleComponent* Capsule = Owner->FindComponentByClass<UCapsuleComponent>();
        UPrimitiveComponent* MoveComp = Capsule ? Cast<UPrimitiveComponent>(Capsule)
                                                : Cast<UPrimitiveComponent>(Owner->GetRootComponent());

        if (!MoveComp) return;
        
        if (Capsule && GetWorld())
        {
            
            if (GLaunchDebugAccum == 0.0f) // only when we logged above
            {
                const FVector ActorLoc = Owner->GetActorLocation();
                const FVector RootLoc  = Owner->GetRootComponent() ? Owner->GetRootComponent()->GetComponentLocation() : FVector::ZeroVector;
                const FVector FwdLoc   = ForwardComp ? ForwardComp->GetComponentLocation() : FVector::ZeroVector;

                UE_LOG(LogTemp, Warning,
                    TEXT("[LaunchDebug] ActorLoc=%s RootLoc=%s ForwardCompLoc=%s"),
                    *ActorLoc.ToString(),
                    *RootLoc.ToString(),
                    *FwdLoc.ToString()
                );
            }

            // Draw actor location (white)
            DrawDebugSphere(GetWorld(), Owner->GetActorLocation(), 6.f, 8, FColor::White, false, 0.0f);

            // Draw ForwardComp location (cyan) 
            if (ForwardComp)
            {
                DrawDebugSphere(GetWorld(), ForwardComp->GetComponentLocation(), 6.f, 8, FColor::Cyan, false, 0.0f);
            }

            // camera component on the pawn (yellow)
            if (UCameraComponent* Cam = Owner->FindComponentByClass<UCameraComponent>())
            {
                DrawDebugSphere(GetWorld(), Cam->GetComponentLocation(), 6.f, 8, FColor::Yellow, false, 0.0f);
                DrawDebugLine(GetWorld(), Cam->GetComponentLocation(), Cam->GetComponentLocation() + Cam->GetForwardVector() * 30.f, FColor::Yellow, false, 0.0f, 0, 1.0f);
            }

            // Draw hands (red/blue)
            if (LeftHand)
            {
                DrawDebugSphere(GetWorld(), LeftHand->GetComponentLocation(), 5.f, 8, FColor::Red, false, 0.0f);
            }
            if (RightHand)
            {
                DrawDebugSphere(GetWorld(), RightHand->GetComponentLocation(), 5.f, 8, FColor::Blue, false, 0.0f);
            }
        }
        
        const int32 NumSteps = 30;
        const float StepDT = DeltaTime / NumSteps;

        for (int32 i = 0; i < NumSteps; ++i)
        {
            FVector StepDelta = LaunchVelocity * StepDT;
            
            MoveComp->AddWorldOffset(StepDelta, true, &Hit, ETeleportType::None);

          /*  UE_LOG(LogTemp, Warning, TEXT("Sweep hit? %d  BlockingHit? %d  StartPen=%d"),
                Hit.bBlockingHit ? 1 : 0,
                Hit.IsValidBlockingHit() ? 1 : 0,
                Hit.bStartPenetrating ? 1 : 0
            ); */

            if (Hit.IsValidBlockingHit())
            {
                LaunchVelocity = FVector::ZeroVector;
                bLaunching = false;

            /*    UE_LOG(LogTemp, Warning, TEXT("Launch BLOCKED by %s comp %s normal=%s loc=%s"),
                    Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("None"),
                    Hit.GetComponent() ? *Hit.GetComponent()->GetName() : TEXT("None"),
                    *Hit.Normal.ToString(),
                    *Hit.ImpactPoint.ToString()
                ); */
                break;
            }

            if (!bLaunching)
            {
                break;
            }
        }
        
        if (Capsule)
        {
          /*  UE_LOG(LogTemp, Warning,
                TEXT("Capsule CollisionEnabled=%d Profile=%s ObjType=%d"),
                (int32)Capsule->GetCollisionEnabled(),
                *Capsule->GetCollisionProfileName().ToString(),
                (int32)Capsule->GetCollisionObjectType()
            );

            UE_LOG(LogTemp, Warning,
                TEXT("Capsule responses: WorldStatic=%d WorldDynamic=%d Pawn=%d"),
                (int32)Capsule->GetCollisionResponseToChannel(ECC_WorldStatic),
                (int32)Capsule->GetCollisionResponseToChannel(ECC_WorldDynamic),
                (int32)Capsule->GetCollisionResponseToChannel(ECC_Pawn)
            );*/
        }
        
        if (LaunchVelocity.SizeSquared() < 25.f)
        {
            LaunchVelocity = FVector::ZeroVector;
            bLaunching = false;
        }
    }

    if (!LeftHand || !RightHand) return;

    // If triggers released reset
    if (!AreAllButtonsHeld())
    {
        State = EEarthLaunchState::Idle;
        PrimeRemaining = 0.f;
        return;
    }
    
    const USceneComponent* Ref = ForwardComp ? ForwardComp : GetOwner()->GetRootComponent();
    if (!Ref) return;

    const FVector Fwd = Ref->GetForwardVector().GetSafeNormal();
    const FVector Up = FVector::UpVector;
    
    const FVector LDir = LeftHand->GetForwardVector().GetSafeNormal();
    const FVector RDir = RightHand->GetForwardVector().GetSafeNormal();
    
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
                ApplyLaunch(AvgUpDot);

                State = EEarthLaunchState::Cooldown;
                CooldownRemaining = Cooldown;
            }
            else
            {
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

    const FVector TargetXY = Origin;
    
    const FVector Start = TargetXY + FVector(0, 0, TraceUp);
    const FVector End   = TargetXY - FVector(0, 0, TraceDown);

    FHitResult Hit;
    TArray<AActor*> Ignore;
    Ignore.Add(Owner);
    
    const bool bHit = UKismetSystemLibrary::SphereTraceSingle(
    GetWorld(),
    Start,
    End,
    15.f, 
    UEngineTypes::ConvertToTraceType(ECC_Visibility),
    false,
    Ignore,
    EDrawDebugTrace::ForDuration,
    Hit,
    true
);

    if (!bHit) return false;

    OutLoc = Hit.Location;

    // Face the same yaw as the forward
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

    float UpAmount = (AvgUpDot - LaunchUpMin) / FMath::Max(LaunchUpMax - LaunchUpMin, 0.01f);
    UpAmount = FMath::Clamp(UpAmount, 0.f, 1.f);

    const float UpScale = FMath::Lerp(LaunchUpScaleMin, LaunchUpScaleMax, UpAmount);

    return (Fwd + Up * UpScale).GetSafeNormal();
}

float UEarthLaunchAbilityComponent::GetFastFallT() const
{
    if (!WaistComp || !LeftFootComp || !RightFootComp) return 0.f;

    const float FeetZ = 0.5f * (LeftFootComp->GetComponentLocation().Z + RightFootComp->GetComponentLocation().Z);
    const float WaistZ = WaistComp->GetComponentLocation().Z;

    const float CompressionNow = WaistZ - FeetZ;

    // Positive when crouching lower than baseline
    const float CrouchAmount = StandCompression - CompressionNow;

    float T = (CrouchAmount - FastFallStart) / FMath::Max(FastFallFull - FastFallStart, 1.f);
    T = FMath::Clamp(T, 0.f, 1.f);

    // Smooth a bit so it doesn't feel twitchy
    T = FMath::Pow(T, 0.85f);

    return T;
}

float UEarthLaunchAbilityComponent::ComputeLaunchPowerT() const
{
    if (!LeftVel || !RightVel)
    {
        return 0.f;
    }

    AActor* Owner = GetOwner();
    const USceneComponent* Ref = (Owner && ForwardComp) ? ForwardComp : (Owner ? Owner->GetRootComponent() : nullptr);
    if (!Ref)
    {
        return 0.f;
    }

    const FVector FlatForward = FVector(Ref->GetForwardVector().X, Ref->GetForwardVector().Y, 0.f).GetSafeNormal();
    if (FlatForward.IsNearlyZero())
    {
        return 0.f;
    }

    const FVector AvgVel = 0.5f * (LeftVel->GetVelocity() + RightVel->GetVelocity());
    
    const float ForwardSpeed = FMath::Max(0.f, FVector::DotProduct(AvgVel, FlatForward));

    // Tune these two values
    const float MinCastSpeed = 80.f;
    const float MaxCastSpeed = 1500.f;

    float T = (ForwardSpeed - MinCastSpeed) / FMath::Max(MaxCastSpeed - MinCastSpeed, 1.f);
    T = FMath::Clamp(T, 0.f, 1.f);
    
    T = FMath::Pow(T, 0.85f);

    return T;
}

void UEarthLaunchAbilityComponent::ApplyLaunch(float AvgUpDot)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    const USceneComponent* Ref = ForwardComp ? ForwardComp : Owner->GetRootComponent();
    if (!Ref) return;

    const FVector FlatForward = FVector(Ref->GetForwardVector().X, Ref->GetForwardVector().Y, 0.f).GetSafeNormal();
    const FVector Up = FVector::UpVector;

    // Height from hand angle
    float HeightT = (AvgUpDot - LaunchUpMin) / FMath::Max(LaunchUpMax - LaunchUpMin, 0.01f);
    HeightT = FMath::Clamp(HeightT, 0.f, 1.f);
    HeightT = FMath::Pow(HeightT, 0.9f);

    // Distance from cast speed
    const float PowerT = ComputeLaunchPowerT();
    
    const float HorizontalSpeed = FMath::Lerp(300.f, 4000.f, PowerT);
    const float VerticalSpeed   = FMath::Lerp(500.f, 8000.f, HeightT);

    LaunchVelocity = FlatForward * HorizontalSpeed + Up * VerticalSpeed;
    bLaunching = true;

    UE_LOG(LogTemp, Warning,
        TEXT("Launch HeightT=%.2f PowerT=%.2f HSpeed=%.1f VSpeed=%.1f Vel=%s"),
        HeightT,
        PowerT,
        HorizontalSpeed,
        VerticalSpeed,
        *LaunchVelocity.ToString()
    );
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
    
    AActor* Owner = GetOwner();
    const USceneComponent* Ref = (Owner && ForwardComp) ? ForwardComp : (Owner ? Owner->GetRootComponent() : nullptr);
    const FVector Fwd = Ref ? Ref->GetForwardVector().GetSafeNormal() : FVector::ForwardVector;
    
    const FVector PillarDir = (-Fwd + FVector::UpVector).GetSafeNormal();

    Pillar->InitLaunch(PillarDir);
}