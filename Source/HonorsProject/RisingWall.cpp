// Fill out your copyright notice in the Description page of Project Settings.


#include "RisingWall.h"
#include "TrackedVelocity.h"
#include "TaskManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ARisingWall::ARisingWall()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	Mesh->SetSimulatePhysics(false);
	Mesh->SetEnableGravity(false);
	
	Mesh->SetGenerateOverlapEvents(true);
	Mesh->SetNotifyRigidBodyCollision(true);
}

void ARisingWall::BeginPlay()
{
	Super::BeginPlay();
	
	StartRising();
	
	if (Mesh)
	{
		Mesh->SetGenerateOverlapEvents(true);
		
		Mesh->OnComponentBeginOverlap.AddDynamic(this, &ARisingWall::OnMeshBeginOverlap);
		
		Mesh->OnComponentHit.AddDynamic(this, &ARisingWall::OnMeshHit);
	}
}

void ARisingWall::StartRising()
{
	const FVector StartLoc = GetActorLocation();
	const FVector Target = StartLoc + FVector(0.f, 0.f, RiseHeight);
	
	SetCollisionEnabled(false);

	BeginMoveTo(Target, false);
}

void ARisingWall::StartLoweringAndDestroy()
{
	if (bLowering)
	{
		return;
	}
	
	ATaskManager* TaskManager = Cast<ATaskManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATaskManager::StaticClass()));
	
	bLowering = true;
	const FVector StartLoc = GetActorLocation();
	const FVector Target = StartLoc - FVector(0.f, 0.f, RiseHeight);
	
	SetCollisionEnabled(false);

	BeginMoveTo(Target, true);
	if (TaskManager->IsExpectingWallLowered())
	{
		TaskManager->NotifyWallLowered();
	}
}

void ARisingWall::BeginMoveTo(const FVector& Target, bool bDestroyAfter)
{
	MoveStart = GetActorLocation();
	MoveEnd = Target;
	Elapsed = 0.0f;

	bMoving = true;
	bDestroyWhenDone = bDestroyAfter;

	PrimaryActorTick.bCanEverTick = true;
}

void ARisingWall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!bMoving) return;

	if (RiseTime <= 0.0f)
	{
		SetActorLocation(MoveEnd);
		bMoving = false;

		if (!bDestroyWhenDone && bEnableCollisionWhenFinished)
		{
			SetCollisionEnabled(true);
			Mesh->SetSimulatePhysics(true);
			Mesh->BodyInstance.bLockZTranslation = true;
		}

		if (bDestroyWhenDone)
		{
			Destroy();
		}
		else
		{
			PrimaryActorTick.bCanEverTick = false;
		}
		return;
	}

	Elapsed += DeltaTime;
	const float Alpha = FMath::Clamp(Elapsed / RiseTime, 0.0f, 1.0f);
	
	const float Smooth = Alpha * Alpha * (3.0f - 2.0f * Alpha);

	SetActorLocation(FMath::Lerp(MoveStart, MoveEnd, Smooth));

	if (Alpha >= 1.0f)
	{
		SetActorLocation(MoveEnd);
		
		bMoving = false;
		bCanBreak = true;

		if (!bDestroyWhenDone && bEnableCollisionWhenFinished)
		{
			SetCollisionEnabled(true);
			Mesh->BodyInstance.bLockZTranslation = true;
		}

		if (bDestroyWhenDone)
		{
			Destroy();
		}
		else
		{
			PrimaryActorTick.bCanEverTick = false;
		}
	}
}

void ARisingWall::SetCollisionEnabled(bool bEnabled)
{
	if (!Mesh) return;

	Mesh->SetCollisionEnabled(
		bEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision
	);
}

void ARisingWall::WallBreak()
{
	if (BrokenWallClass && GetWorld())
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* BrokenWall = GetWorld()->SpawnActor<AActor>(
			BrokenWallClass,
			GetActorLocation(),
			GetActorRotation(),
			Params
		);

		if (BrokenWall)
		{
			BrokenWall->SetActorScale3D(GetActorScale3D());
		}
	}

	Destroy();
}

void ARisingWall::OnMeshBeginOverlap(
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
	Dir.Z = 0.0f;
	if (Dir.IsNearlyZero()) return;
	
	Mesh->SetSimulatePhysics(true);
	Mesh->BodyInstance.bLockZTranslation = true;
	
	const FVector WallVel = Mesh->GetPhysicsLinearVelocity();
	const FVector RelVel  = HandVel - WallVel;
	
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
	
	const float	WallMassKg = Mesh->GetMass();
	const float ImpulseMag = WallMassKg * DesiredDeltaV * 8;
	
	Mesh->AddImpulse(Dir * ImpulseMag, NAME_None, false);
	
	Mesh->SetEnableGravity(true);
	
	ATaskManager* TaskManager = Cast<ATaskManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATaskManager::StaticClass()));
	
	if (TaskManager->IsExpectingWallStrike())
	{
		TaskManager->NotifyWallStrike();
	}
	
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

void ARisingWall::OnMeshHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit
)
{
	if (!Mesh || !bCanBreak)
		return;

	if (!OtherActor || OtherActor == this)
		return;
	
	if (OtherComp && (OtherComp->ComponentHasTag(TEXT("VRHand")) || OtherComp->ComponentHasTag(TEXT("VRFoot")) || OtherComp->ComponentHasTag(TEXT("Floor"))))
		return;

	const float ImpactSpeed = Mesh->GetPhysicsLinearVelocity().Size();

	if (ImpactSpeed >= BreakImpactSpeed || OtherComp->ComponentHasTag(TEXT("IncomingRock")))
	{
		bCanBreak = false;
		WallBreak();
	}
}