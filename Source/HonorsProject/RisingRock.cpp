// Fill out your copyright notice in the Description page of Project Settings.


#include "RisingRock.h"
#include "Components/StaticMeshComponent.h"
#include "TrackedVelocity.h"
#include "TimerManager.h"

// Sets default values
ARisingRock::ARisingRock()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	
	Mesh->SetSimulatePhysics(true);
	Mesh->SetEnableGravity(true);
	
	Mesh->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void ARisingRock::BeginPlay()
{
	Super::BeginPlay();
	
	BaseMaxHeight = MaxHeight;
	
	if (Mesh)
    {
        Mesh->SetGenerateOverlapEvents(true);
		
        Mesh->OnComponentBeginOverlap.AddDynamic(this, &ARisingRock::OnMeshBeginOverlap);
    }
}

void ARisingRock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bHasLaunched || !Mesh) return;
	
    FVector Loc = GetActorLocation();
    if (Loc.Z > MaxZ)
    {
        Loc.Z = MaxZ;
        SetActorLocation(Loc);

        // Stop upward motion if it hits the cap
        FVector V = Mesh->GetPhysicsLinearVelocity();
        if (V.Z > 0.0f)
        {
            V.Z = 0.0f;
            Mesh->SetPhysicsLinearVelocity(V);
        }
    }
}

void ARisingRock::SetSizeScale(float Scale)
{
	MaxHeight = BaseMaxHeight * Scale;
}

void ARisingRock::LaunchFromStomp(float StompStrength, float MaxAllowedWorldZ)
{
    if (!Mesh) return;
	
	Mesh->SetUseCCD(false);

    MaxZ = MaxAllowedWorldZ - MaxHeight;
	
    // Convert stomp strength to an upward impulse
    const float RawImpulse = (StompStrength * StrengthToImpulse);
    const float UpImpulse = FMath::Clamp(RawImpulse, MinUpImpulse, MaxUpImpulse);
	
    FVector V = Mesh->GetPhysicsLinearVelocity();
    V.Z = 0.f;
    Mesh->SetPhysicsLinearVelocity(V);

    Mesh->AddImpulse(FVector(0.f, 0.f, UpImpulse), NAME_None, true);

    bHasLaunched = true;
	
	GetWorldTimerManager().ClearTimer(CCDEnableTimer);
	GetWorldTimerManager().SetTimer(
		CCDEnableTimer,
		this,
		&ARisingRock::EnableCCD,
		0.5f,
		false
	);
}

void ARisingRock::EnableCCD()
{
	if (Mesh)
	{
		Mesh->SetUseCCD(true);
	}
}

void ARisingRock::OnMeshBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!bCanBePunched || !Mesh || !OtherComp) return;
	
	if (Mesh->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
		return;
	
	const bool bIsHand = OtherComp->ComponentHasTag(TEXT("VRHand"));
	const bool bIsFoot = OtherComp->ComponentHasTag(TEXT("VRFoot"));
	
	if (!bIsHand && !bIsFoot)
		return;

	FVector HandVel = FVector::ZeroVector;
	
	TArray<UTrackedVelocity*> VelComps;
	AActor* HandOwner = OtherComp->GetOwner();
	if (HandOwner)
	{
		const bool bIsLeft  = OtherComp->ComponentHasTag(TEXT("LeftHand"));
		const bool bIsRight = OtherComp->ComponentHasTag(TEXT("RightHand"));
		const bool bIsLeftFoot  = OtherComp->ComponentHasTag(TEXT("LeftFoot"));
		const bool bIsRightFoot = OtherComp->ComponentHasTag(TEXT("RightFoot"));

		HandOwner->GetComponents<UTrackedVelocity>(VelComps);
		for (UActorComponent* C : VelComps)
		{
			UTrackedVelocity* TV = Cast<UTrackedVelocity>(C);
			if (!TV) continue;

			if (bIsLeft && TV->ComponentHasTag(TEXT("LeftVel")))
			{
				HandVel = TV->GetVelocity();
				break;
			}
			if (bIsRight && TV->ComponentHasTag(TEXT("RightVel")))
			{
				HandVel = TV->GetVelocity();
				break;
			}
			if (bIsLeftFoot && TV->ComponentHasTag(TEXT("LeftFootVel")))
			{
				HandVel = TV->GetVelocity();
				break;
			}
			if (bIsRightFoot && TV->ComponentHasTag(TEXT("RightFootVel")))
			{
				HandVel = TV->GetVelocity();
				break;
			}
		}
	}

	// If velocity is unreliable
	if (HandVel.SizeSquared() < 25.0f)
	{
		HandVel = (GetActorLocation() - OtherComp->GetComponentLocation()) * 10.0f;
	}

	FVector Dir = HandVel.GetSafeNormal();
	if (Dir.IsNearlyZero()) return;
	
	Mesh->SetSimulatePhysics(true);
	
	const FVector RockVel = Mesh->GetPhysicsLinearVelocity();
	const FVector RelVel  = HandVel - RockVel;
	
	const float ClosingSpeed = FVector::DotProduct(RelVel, Dir); 
	if (ClosingSpeed < PunchMinSpeed) return;
	
	float Alpha = (ClosingSpeed - PunchMinSpeed) / FMath::Max(PunchMaxSpeed - PunchMinSpeed, 1.f);
	Alpha = FMath::Clamp(Alpha, 0.f, 1.f);
	
	Alpha = FMath::Pow(Alpha, PunchPower);
	
	float DesiredDeltaV = FMath::Lerp(MinDeltaV, MaxDeltaV, Alpha) * MomentumTransfer;
	
	if (DesiredDeltaV > 450)
	{
		DesiredDeltaV *= 2;
	}

	// Convert to impulse 
	const float RockMassKg = Mesh->GetMass();
	const float ImpulseMag = RockMassKg * DesiredDeltaV * 2;
	
	Mesh->AddImpulse(Dir * ImpulseMag, NAME_None, false);
	
	Mesh->SetEnableGravity(true);
	
	UE_LOG(LogTemp, Warning, TEXT("ClosingSpeed=%.0f  AlphaRaw=%.2f  Alpha=%.2f  DesiredDV=%.0f"),
	ClosingSpeed,
	(ClosingSpeed - PunchMinSpeed) / FMath::Max(PunchMaxSpeed - PunchMinSpeed, 1.f),
	Alpha,
	DesiredDeltaV
);

	// Cooldown 
	bCanBePunched = false;
	GetWorldTimerManager().SetTimer(
		PunchCooldownTimer,
		[this]()
		{
			bCanBePunched = true;
		},
		PunchCooldown,
		false
	);
}