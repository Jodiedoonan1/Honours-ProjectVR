// Fill out your copyright notice in the Description page of Project Settings.


#include "RockLauncher.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "IncomingRock.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

// Sets default values
ARockLauncher::ARockLauncher()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
	MuzzlePoint->SetupAttachment(SceneRoot);
}

// Called when the game starts or when spawned
void ARockLauncher::BeginPlay()
{
	Super::BeginPlay();
	
}

void ARockLauncher::FireRock()
{
	UE_LOG(LogTemp, Warning, TEXT("%s: FireRock called"), *GetName());
	
	if (!ProjectileClass || !GetWorld() || !MuzzlePoint)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PlayerController)
	{
		return;
	}

	APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
	if (!CameraManager)
	{
		return;
	}

	const FVector SpawnLocation = MuzzlePoint->GetComponentLocation();
	const FVector TargetLocation = CameraManager->GetCameraLocation() + FVector(0.f, 0.f, AimHeightOffset);
	const FVector Direction = (TargetLocation - SpawnLocation).GetSafeNormal();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
//	DrawDebugLine(GetWorld(), SpawnLocation, TargetLocation, FColor::Red, false, 2.0f, 0, 2.0f);
//	DrawDebugDirectionalArrow(GetWorld(), SpawnLocation, SpawnLocation + (Direction * 300.f), 50.f, FColor::Green, false, 2.0f, 0, 2.0f);

	AIncomingRock* Projectile = GetWorld()->SpawnActor<AIncomingRock>(
		ProjectileClass,
		SpawnLocation,
		Direction.Rotation(),
		Params
	);

	if (Projectile)
	{
		Projectile->LaunchInDirection(Direction);
	}
}

