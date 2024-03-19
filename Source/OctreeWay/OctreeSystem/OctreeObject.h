
#pragma once

#include "CoreMinimal.h"
#include "Structs/Uint64ToObjectPtrArray.h"
#include "OctreeObject.generated.h"


UCLASS()
class OCTREEWAY_API UOctreeObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Octree")
	bool IsFree = true;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Octree")
	int32 Depth = -1;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Octree")
	FVector CenterNodeLocation;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Octree")
	TArray<UOctreeObject*> Children;

	FUint64ToObjectPtrArray ChildrenMortonOctreeArray;
};
