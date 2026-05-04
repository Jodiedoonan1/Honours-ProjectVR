// Fill out your copyright notice in the Description page of Project Settings.


#include "RockTarget.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "RisingRock.h"
#include "RisingWall.h"
#include "Kismet/GameplayStatics.h"
#include "TaskManager.h"

// Sets default values
ARockTarget::ARockTarget()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SceneRoot);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(SceneRoot);
	TriggerBox->SetGenerateOverlapEvents(true);
}

void ARockTarget::BeginPlay()
{
	Super::BeginPlay();
	
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ARockTarget::OnTargetOverlap);
}

void ARockTarget::OnTargetOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!OtherActor)
	{
		return;
	}

	ARisingRock* Rock = Cast<ARisingRock>(OtherActor);
	ARisingWall* Wall = Cast<ARisingWall>(OtherActor);
	if (!Rock && !Wall)
	{
		return;
	}

	ATaskManager* TaskManager = Cast<ATaskManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ATaskManager::StaticClass()));

	if (TaskManager->IsExpectingTargetTask())
	{
		TaskManager->NotifyTargetHit();
	}
	
	if (TaskManager->IsExpectingSpawnerDestroyed())
	{
		TaskManager->NotifySpawnerDestroyed();
	}

	Destroy();
}