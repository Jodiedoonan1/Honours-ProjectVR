// Fill out your copyright notice in the Description page of Project Settings.


#include "RisingRock.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

// Sets default values
ARisingRock::ARisingRock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	
	Mesh->SetSimulatePhysics(false);
	Mesh->SetEnableGravity(false);
	
	Mesh->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void ARisingRock::BeginPlay()
{
	Super::BeginPlay();
	
	StartLoc = GetActorLocation();
	EndLoc = StartLoc + FVector(0.f, 0.f, RiseHeight);

	// Disable collision while rising
	SetCollisionEnabled(false);
	
	if (Mesh)
	{
		Mesh->OnComponentBeginOverlap.AddDynamic(this, &ARisingRock::OnMeshBeginOverlap);
	}
	
}

// Called every frame
void ARisingRock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (RiseTime <= 0.f)
	{
		SetActorLocation(EndLoc);
		if (bEnableCollision) SetCollisionEnabled(true);
		PrimaryActorTick.bCanEverTick = false;
		return;
	}

	Elapsed += DeltaTime;
	const float Alpha = FMath::Clamp(Elapsed / RiseTime, 0.f, 1.f);

	// Smoothstep (nice ease-in/out)
	const float Smooth = Alpha * Alpha * (3.f - 2.f * Alpha);

	SetActorLocation(FMath::Lerp(StartLoc, EndLoc, Smooth));

	if (Alpha >= 1.f)
	{
		if (bEnableCollision) SetCollisionEnabled(true);
		PrimaryActorTick.bCanEverTick = false;
	}
}

void ARisingRock::SetCollisionEnabled(bool bEnabled)
{
	if (!Mesh) return;
	Mesh->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
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

	// Only respond once it has finished rising (collision enabled)
	if (Mesh->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
		return;

	// Only accept overlaps from our VR hand colliders
	if (!OtherComp->ComponentHasTag(TEXT("VRHand")))
		return;

	// Use hand velocity for punch direction
	FVector Vel = OtherComp->GetComponentVelocity();

	// If velocity is unreliable, fallback to "from hand to rock"
	if (Vel.SizeSquared() < 25.f)
	{
		Vel = (GetActorLocation() - OtherComp->GetComponentLocation()) * 10.f;
	}

	FVector Dir = Vel.GetSafeNormal();
	if (Dir.IsNearlyZero()) return;

	// Turn physics on at the moment of impact
	Mesh->SetSimulatePhysics(true);
	Mesh->SetEnableGravity(false); // keep gravity off for now

	// Apply impulse in punch direction
	Mesh->AddImpulse(Dir * PunchImpulseStrength, NAME_None, true);

	// Simple cooldown so it doesn’t spam impulses each frame while overlapping
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