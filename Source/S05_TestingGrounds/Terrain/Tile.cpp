// Fill out your copyright notice in the Description page of Project Settings.


#include "Tile.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "InfiniteTerrainGameMode.h"
#include "ActorPool.h"
#include "Engine/World.h"
#include "NavigationSystem.h"



// Sets default values
ATile::ATile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MinExtent = FVector(0, -2000, 0);

	MaxExtent = FVector(4000, 2000, 0);

	NavigationBoundsOffset = FVector(2000, 0, 0);
}


void ATile::SetPool(UActorPool* InPool)
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] Setting Pool: %s"), *(this->GetName()), *(InPool->GetName()));
	Pool = InPool;

	PositionNavMeshBoundsVolume();
}

void ATile::PositionNavMeshBoundsVolume()
{
	NavMeshBoundsVolume = Pool->Checkout();

	if (NavMeshBoundsVolume == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Not Enough Actors in the Pool."), *GetName());
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("[%s] CheckedOut"), *GetName(), *NavMeshBoundsVolume->GetName());
	NavMeshBoundsVolume->SetActorLocation(GetActorLocation() + NavigationBoundsOffset);
	FNavigationSystem::Build(*GetWorld());

}

void ATile::PlaceActors(TSubclassOf<AActor> ToSpawn, int MinSpawn, int MaxSpawn, float Radius, float MinScale, float MaxScale)
{
	int NumberToSpawn(FMath::RandRange(MinSpawn, MaxSpawn));
	for (size_t i = 0; i < NumberToSpawn; i++)
	{
		FVector SpawnPoint;
		float RandomScale = FMath::RandRange(MinScale, MaxScale);
		bool found = FindEmptyLocation(SpawnPoint, Radius * RandomScale);
		if (found)
		{
			float RandomRotation = FMath::RandRange(-180.f, 180.f);
			
			PlaceActor(ToSpawn, SpawnPoint, RandomRotation, RandomScale);
		}
		
	}


}

bool ATile::FindEmptyLocation(FVector &OutLocation, float Radius)
{
	
	FBox Bounds(MinExtent, MaxExtent);
	

	const int MAX_ATTEMPTS = 100;
	for (size_t i = 0; MAX_ATTEMPTS; i++)
	{
		FVector CandidatePoint = FMath::RandPointInBox(FBox(Bounds));;

		if (CanSpawnAtLocation(CandidatePoint, Radius))
		{
			OutLocation = CandidatePoint;
			return true;
		}
	}
	return false;
	
	
}
void ATile::PlaceActor(TSubclassOf<AActor> ToSpawn, FVector SpawnPoint, float Rotation, float Scale)
{
	AActor* Spawned = GetWorld()->SpawnActor<AActor>(ToSpawn);
	Spawned->SetActorRelativeLocation(SpawnPoint);
	Spawned->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
	Spawned->SetActorRotation(FRotator(0, Rotation, 0));
	Spawned->SetActorScale3D(FVector(Scale));
}

// Called when the game starts or when spawned
void ATile::BeginPlay()
{
	Super::BeginPlay();



}

void ATile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	Pool->Return(NavMeshBoundsVolume);
}
// Called every frame
void ATile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


bool ATile::CanSpawnAtLocation(FVector Location, float Radius)
{
	FHitResult HitResult;
	FVector GlobalLocation = ActorToWorld().TransformPosition(Location);
	bool HasHit = GetWorld()->SweepSingleByChannel(HitResult, GlobalLocation, GlobalLocation, FQuat::Identity,
	ECollisionChannel::ECC_GameTraceChannel2, FCollisionShape::MakeSphere(Radius));
	
	return !HasHit;


}
