
#include "OctreeBuildBoxPathfinding.h"

#include "EngineUtils.h"
#include "Engine/TargetPoint.h"
#include "OctreeWay/Utils/FuncLib.h"
#include "OctreeWay/Utils/Morton.h"

void AOctreeBuildBoxPathfinding::BeginPlay()
{
	Super::BeginPlay();
	
	FindTargetOctree();
}


void AOctreeBuildBoxPathfinding::ProcessGenerateOctrees(UOctreeObject* NewOctree)
{
	if (NewOctree->Depth != 0 || !NewOctree->IsFree) return;
	
	const uint64 MortonCode = Morton::FVectorToMorton(NewOctree->CenterNodeLocation);
	RootMortonOctreeArray.Add(MortonCode, NewOctree);
}

void AOctreeBuildBoxPathfinding::FindClosestChildrenToLocation(UOctreeObject* Octree, const uint64 MortonLoc)
{
	const FVector LocationOctreeParamFunc = Octree->CenterNodeLocation;
	if (Octree->Children.IsEmpty())
	{
		FuncLib::DrawDebug3DCrosshair(GetWorld(), LocationOctreeParamFunc, 10000, FColor::Blue, 100, 10);
		return;
	}

	const TObjectPtr<UOctreeObject> OctreeFindClosest = Octree->ChildrenMortonOctreeArray.FindClosest(MortonLoc);
	
	const FVector TargetLocation = Morton::MortonToFVector(MortonLoc);
	const float DistOctreeNextClosest = FVector::Dist(OctreeFindClosest->CenterNodeLocation, TargetLocation);
	const float DistOctreeParamFunc = FVector::Dist(Octree->CenterNodeLocation, TargetLocation);
	if (DistOctreeNextClosest < DistOctreeParamFunc)
	{
		FindClosestChildrenToLocation(OctreeFindClosest, MortonLoc);
	}
	else
	{
		FuncLib::DrawDebug3DCrosshair(GetWorld(), LocationOctreeParamFunc, 10000, FColor::Blue, 100, 10);
	}
}

void AOctreeBuildBoxPathfinding::FindTargetOctree()
{
	for (TActorIterator<ATargetPoint> It(GetWorld()); It; ++It)
	{
		if (const ATargetPoint* TargetPoint = *It)
		{
			const FVector TargetLocation = TargetPoint->GetActorLocation();
			const uint64 MortonLoc = Morton::FVectorToMorton(TargetLocation);

			const TObjectPtr<UOctreeObject> RootOctreeFind = RootMortonOctreeArray.FindClosest(MortonLoc);
			if (IsValid(RootOctreeFind))
			{
				//FuncLib::DrawDebug3DCrosshair(GetWorld(), Morton::MortonToFVector(MortonLoc), 10000, FColor::Orange, 100, 10);
				//FuncLib::DrawDebug3DCrosshair(GetWorld(), RootOctreeFind->CenterNodeLocation, 10000, FColor::Red, 100, 30);
				//FindClosestChildrenToLocation(RootOctreeFind, MortonLoc);
			}
			return;
		}
	}
}
