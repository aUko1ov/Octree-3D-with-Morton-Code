
#pragma once

#include "CoreMinimal.h"
#include "OctreeBuildBox.h"
#include "OctreeBuildBoxPathfinding.generated.h"


UCLASS()
class OCTREEWAY_API AOctreeBuildBoxPathfinding : public AOctreeBuildBox
{
	GENERATED_BODY()

protected:
	FUint64ToObjectPtrArray RootMortonOctreeArray;

	virtual void BeginPlay() override;
	virtual void ProcessGenerateOctrees(UOctreeObject* NewOctree) override;
	
	void FindClosestChildrenToLocation(UOctreeObject* Octree, const uint64 MortonLoc);
	
	void FindTargetOctree();
};
