// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectiveMarker.h"
#include "Kismet/GameplayStatics.h"
#include "TaskManager.h"

AObjectiveMarker::AObjectiveMarker()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	Decal = CreateDefaultSubobject<UDecalComponent>(TEXT("Decal"));
	Decal->SetupAttachment(SceneRoot);
	Decal->DecalSize = FVector(128.f, 256.f, 256.f);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(SceneRoot);
	TriggerBox->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	TriggerBox->SetGenerateOverlapEvents(true);
}

void AObjectiveMarker::BeginPlay()
{
	Super::BeginPlay();
	
	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AObjectiveMarker::OnMarkerOverlap);
	}
}

void AObjectiveMarker::OnMarkerOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (bAlreadyTriggered || !OtherActor)
	{
		return;
	}

	ATaskManager* TaskManager = Cast<ATaskManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ATaskManager::StaticClass()));

	if (!TaskManager)
	{
		return;
	}

	if (OtherActor == UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		bAlreadyTriggered = true;
		TaskManager->NotifyObjectiveMarkerReached();
		Destroy();
	}
}