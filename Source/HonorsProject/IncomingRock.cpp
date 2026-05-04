// Fill out your copyright notice in the Description page of Project Settings.


#include "IncomingRock.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TaskManager.h"

// Sets default values
AIncomingRock::AIncomingRock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	
	Mesh->SetSimulatePhysics(false);
	Mesh->SetEnableGravity(false);
	Mesh->SetNotifyRigidBodyCollision(true);
	Mesh->SetGenerateOverlapEvents(true);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 1200.0f;
	ProjectileMovement->MaxSpeed = 1200.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
}

// Called when the game starts or when spawned
void AIncomingRock::BeginPlay()
{
	Super::BeginPlay();
	
	if (Mesh)
	{
		Mesh->OnComponentHit.AddDynamic(this, &AIncomingRock::OnProjectileHit);
	
		Mesh->OnComponentBeginOverlap.AddDynamic(this, &AIncomingRock::OnMeshBeginOverlap);
	}
}

void AIncomingRock::LaunchInDirection(const FVector& Direction)
{
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = Direction.GetSafeNormal() * ProjectileMovement->InitialSpeed;
	}
}

void AIncomingRock::OnProjectileHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit
)
{
	if (bAlreadyResolved)
	{
		return;
	}

	bAlreadyResolved = true;

	const bool bHitWall =
		(OtherActor && OtherActor->ActorHasTag(TEXT("EarthWall"))) ||
		(OtherComp && OtherComp->ComponentHasTag(TEXT("EarthWall")));

	if (bHitWall)
	{
		ATaskManager* TaskManager = Cast<ATaskManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), ATaskManager::StaticClass()));

		if (TaskManager)
		{
			TaskManager->NotifyBlockedRock();
		}
	}

	Destroy();
}

void AIncomingRock::OnMeshBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	const bool HitPlayer = 
		(OtherActor && OtherActor->ActorHasTag(TEXT("Player"))) ||
		(OtherComp && OtherComp->ComponentHasTag(TEXT("Player")));
	
	if (HitPlayer)
	{
		ATaskManager* TaskManager = Cast<ATaskManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), ATaskManager::StaticClass()));
		if (TaskManager->IsExpectingSpawnerDestroyed())
		{
			TaskManager->PlayerHitReset();
		}
	}
}