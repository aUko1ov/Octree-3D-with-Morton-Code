
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OctreeWay/OctreeSystem/OctreeObject.h"
#include "OctreeBuildBox.generated.h"

UENUM(BlueprintType)
enum class EOctreeDrawState : uint8
{
	FreeOctree UMETA(DisplayName = "Free Octree"),
	BlockedOctree UMETA(DisplayName = "Blocked Octree"),
};


UCLASS()
class OCTREEWAY_API AOctreeBuildBox : public AActor
{
	GENERATED_BODY()
	
protected:
	AOctreeBuildBox();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (EditCondition = "GenerationStarted==false"))
	class UBoxComponent* BuildBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "OctreeWay", meta = (EditCondition = "GenerationStarted==false"))
	TEnumAsByte<ECollisionChannel> TraceChannel = ECollisionChannel::ECC_Visibility;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "OctreeWay", meta = (EditCondition = "GenerationStarted==false", ClampMin = "0.1", UIMin = "0.1"))
	float SmallestNodeSize = 120;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OctreeWay|Render")
	float DebugBoxesThickness = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OctreeWay|Render")
	FColor ColorDrawOctree = FColor::Green;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OctreeWay|Render")
	EOctreeDrawState OctreeDrawState = EOctreeDrawState::FreeOctree;
	
	UFUNCTION(BlueprintCallable, Category = "OctreeWay|Render")
	void DrawDebugNodesAroundLocation(FVector WorldLocation, int VoxelLimit, float Duration);
	void DrawColorBox(const UOctreeObject* Octree, float Duration) const;

	void CollectChildren(UOctreeObject* Octree, TArray<UOctreeObject*>& RelevantOctrees);

	TFuture<TArray<UOctreeObject*>> CollectAndSortOctreesAsync(FVector WorldLocation, int VoxelLimit);

	virtual void ProcessGenerateOctrees(UOctreeObject* NewOctree) {}
	
	bool GenerateGraph();
	void MakeOctreeTree(uint32 RootOctreeIndex);
	
	virtual void BeginPlay() override;
	
	UPROPERTY()
	TArray<UOctreeObject*> Octrees;
	
	FVector StartPosition;
	
	uint32 CountRootNodes_XYZ[3];

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OctreeWay|Render")
	int32 OctreeDepth = 3;

	inline static const FVector ChildPositionOffsetByIndex[8] = {
		{-1, -1, -1},
		{-1,  1, -1},
		{-1, -1,  1},
		{-1,  1,  1},

		{1, -1, -1},
		{1,  1, -1},
		{1, -1,  1},
		{1,  1,  1}
	};
	
	float LookupTable_VoxelSizeByDepth[3];
};

