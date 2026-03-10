// Fill out your copyright notice in the Description page of Project Settings.


#include "LaunchPillar.h"

// Sets default values
ALaunchPillar::ALaunchPillar()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	Mesh->SetSimulatePhysics(false);
	Mesh->SetEnableGravity(false);
	
	Mesh->SetCollisionProfileName(UCollisionProfile::CustomCollisionProfileName);
	
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);     
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); 

	SetCollisionEnabled(false);

}

// Called when the game starts or when spawned
void ALaunchPillar::BeginPlay()
{
	Super::BeginPlay();
	
}

void ALaunchPillar::InitLaunch(const FVector& InLaunchDir)
{
	LaunchDir = InLaunchDir.GetSafeNormal();
	if (LaunchDir.IsNearlyZero())
	{
		LaunchDir = FVector::UpVector;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("LaunchPillar: InitLaunch %s InDir=%s Normalized=%s"),
	*GetName(), *InLaunchDir.ToString(), *LaunchDir.ToString());

	BeginRise();
}

void ALaunchPillar::BeginRise()
{
	MoveStart = GetActorLocation();
	MoveEnd = MoveStart + LaunchDir * RiseDistance;
	Elapsed = 0.f;
	bMoving = true;

	SetCollisionEnabled(false);
	
	SetLifeSpan(LifeAfterRise);
}

// Called every frame
void ALaunchPillar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!bMoving) return;

	if (RiseTime <= KINDA_SMALL_NUMBER)
	{
		SetActorLocation(MoveEnd);
		bMoving = false;

		if (bEnableCollisionWhenFinished)
		{
			SetCollisionEnabled(true);
		}

		if (bAutoDestroy)
		{
			SetLifeSpan(LifeAfterRise);
		}
		else
		{
			PrimaryActorTick.bCanEverTick = false;
		}
		return;
	}

	Elapsed += DeltaTime;
	const float Alpha = FMath::Clamp(Elapsed / RiseTime, 0.f, 1.f);
	
	const float Smooth = Alpha * Alpha * (3.f - 2.f * Alpha);

	SetActorLocation(FMath::Lerp(MoveStart, MoveEnd, Smooth));

	if (Alpha >= 1.f)
	{
		SetActorLocation(MoveEnd);
		bMoving = false;

		if (bEnableCollisionWhenFinished)
		{
			SetCollisionEnabled(true);
		}

		if (bAutoDestroy)
		{
			SetLifeSpan(LifeAfterRise);
		}
		else
		{
			PrimaryActorTick.bCanEverTick = false;
		}
	}
}

void ALaunchPillar::SetCollisionEnabled(bool bEnabled)
{
	if (!Mesh) return;

	if (bEnabled)
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	else
	{
		Mesh->SetCollisionEnabled(bTraceableWhileMoving ? ECollisionEnabled::QueryOnly
														: ECollisionEnabled::NoCollision);
	}
}