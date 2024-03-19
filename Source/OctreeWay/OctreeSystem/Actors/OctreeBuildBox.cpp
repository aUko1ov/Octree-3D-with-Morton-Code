
#include "OctreeBuildBox.h"
#include "Components/BoxComponent.h"
#include "OctreeWay/Utils/Morton.h"

AOctreeBuildBox::AOctreeBuildBox()
{
	PrimaryActorTick.bCanEverTick = true;
	BuildBox = CreateDefaultSubobject<UBoxComponent>("BuildBox");
	SetRootComponent(BuildBox);
}

TFuture<TArray<UOctreeObject*>> AOctreeBuildBox::CollectAndSortOctreesAsync(FVector WorldLocation, int VoxelLimit)
{
    return Async(EAsyncExecution::ThreadPool, [this, WorldLocation, VoxelLimit]() -> TArray<UOctreeObject*>
    {
        TArray<UOctreeObject*> RelevantOctrees;
        
        // Собираем все объекты для возможной сортировки
        for (UOctreeObject* Octree : Octrees)
        {
            if (Octree)
            {
                RelevantOctrees.Add(Octree);
                // Добавляем дочерние элементы рекурсивно
                CollectChildren(Octree, RelevantOctrees);
            }
        }
        
        // Сортируем все найденные объекты по дистанции
        RelevantOctrees.Sort([&WorldLocation](const UOctreeObject& A, const UOctreeObject& B) {
            return FVector::DistSquared(A.CenterNodeLocation, WorldLocation) < FVector::DistSquared(B.CenterNodeLocation, WorldLocation);
        });

        // Ограничиваем количество объектов до VoxelLimit
        if(RelevantOctrees.Num() > VoxelLimit)
        {
            RelevantOctrees.SetNum(VoxelLimit);
        }

        return RelevantOctrees;
    });
}

void AOctreeBuildBox::CollectChildren(UOctreeObject* Octree, TArray<UOctreeObject*>& RelevantOctrees)
{
    for (UOctreeObject* Child : Octree->Children)
    {
        if (Child)
        {
            RelevantOctrees.Add(Child);
            // Рекурсивно собираем дочерние элементы
            CollectChildren(Child, RelevantOctrees);
        }
    }
}

void AOctreeBuildBox::DrawDebugNodesAroundLocation(FVector WorldLocation, int VoxelLimit, float Duration)
{
    if (VoxelLimit <= 0 || Octrees.IsEmpty())
        return;

    const TFuture<TArray<UOctreeObject*>> FutureRelevantOctrees = CollectAndSortOctreesAsync(WorldLocation, VoxelLimit);
    FutureRelevantOctrees.Wait();
	
    TArray<UOctreeObject*> SortedOctrees = FutureRelevantOctrees.Get();
	
    for (const UOctreeObject* Octree : SortedOctrees)
    {
    	DrawColorBox(Octree, Duration);
    }
}

void AOctreeBuildBox::DrawColorBox(const UOctreeObject* Octree, float Duration) const
{
	const float ExtentSize = LookupTable_VoxelSizeByDepth[Octree->Depth] / 2.0f;
	const FVector BoxExtent = FVector(ExtentSize);
	
	switch(OctreeDrawState)
	{
	case EOctreeDrawState::BlockedOctree:
		if (!Octree->IsFree && Octree->Depth == 3)
		{
			DrawDebugBox(GetWorld(), Octree->CenterNodeLocation, BoxExtent, ColorDrawOctree, false, Duration, 0, DebugBoxesThickness);
		}
		break;
	case EOctreeDrawState::FreeOctree:
	default:
		if (Octree->IsFree)
		{
			DrawDebugBox(GetWorld(), Octree->CenterNodeLocation, BoxExtent, ColorDrawOctree, false, Duration, 0, DebugBoxesThickness);
		}
		break;
	}
}


void AOctreeBuildBox::BeginPlay()
{
	Super::BeginPlay();

	BuildBox->SetCollisionResponseToChannel(TraceChannel, ECR_Ignore);

	GenerateGraph();
}

bool AOctreeBuildBox::GenerateGraph()
{
	UBoxComponent* tempBox = Cast<UBoxComponent>(GetRootComponent());
	tempBox->UpdateOverlaps();

	const float RootNodeSize = SmallestNodeSize * FMath::Pow(2.f, OctreeDepth);

	const float VolumeX = BuildBox->GetScaledBoxExtent().X * 2.0;
	const float VolumeY = BuildBox->GetScaledBoxExtent().Y * 2.0;
	const float VolumeZ = BuildBox->GetScaledBoxExtent().Z * 2.0;

	CountRootNodes_XYZ[0] = FMath::CeilToInt(VolumeX / RootNodeSize);
	CountRootNodes_XYZ[1] = FMath::CeilToInt(VolumeY / RootNodeSize);
	CountRootNodes_XYZ[2] = FMath::CeilToInt(VolumeZ / RootNodeSize);

	for (int i = 0; i <= OctreeDepth; i++)
	{
		LookupTable_VoxelSizeByDepth[i] = SmallestNodeSize * FMath::Pow(2.f, OctreeDepth - i);
	}

	StartPosition = GetActorLocation() - BuildBox->GetScaledBoxExtent() + LookupTable_VoxelSizeByDepth[0] / 2;

	const uint32 NumberRootNodes = CountRootNodes_XYZ[0] * CountRootNodes_XYZ[1] * CountRootNodes_XYZ[2];

	Octrees.Reserve(NumberRootNodes);

	for (uint32 Z = 0; Z < CountRootNodes_XYZ[2]; ++Z)
	{
		for (uint32 Y = 0; Y < CountRootNodes_XYZ[1]; ++Y)
		{
			for (uint32 X = 0; X < CountRootNodes_XYZ[0]; ++X)
			{
				UOctreeObject* NewOctree = NewObject<UOctreeObject>();
				const FVector NodePosition = StartPosition + FVector(X, Y, Z) * RootNodeSize;
				NewOctree->CenterNodeLocation = NodePosition;
				
				Octrees.Add(NewOctree);

				ProcessGenerateOctrees(NewOctree);
			}
		}
	}

	const int MaxThreads = (FPlatformMisc::NumberOfCores() - 1);
	const uint32 NodesPerThread = NumberRootNodes / MaxThreads;

	TArray<TFuture<void>> Futures;

	for (int CurrentThread = 0; CurrentThread < MaxThreads; CurrentThread++)
	{
		uint32 StartIndex = NodesPerThread * CurrentThread;
		uint32 EndIndex = (CurrentThread == MaxThreads - 1) ? NumberRootNodes : StartIndex + NodesPerThread;
		
		Futures.Add(Async(EAsyncExecution::ThreadPool, [this, StartIndex, EndIndex]()
			{
				for (uint32 RootOctreeIndex = StartIndex; RootOctreeIndex < EndIndex; ++RootOctreeIndex)
				{
					MakeOctreeTree(RootOctreeIndex);
				}
			}));
	}

	for (auto& Future : Futures)
	{
		Future.Wait();
	}

	return true;
}

void AOctreeBuildBox::MakeOctreeTree(uint32 RootOctreeIndex)
{
    // Локальная функция для рекурсивного обновления узлов
    TFunction<bool(UOctreeObject*, const FVector&, uint32)> updateOctreeRecursively;
	
    updateOctreeRecursively = [&](UOctreeObject* OctreeRef, const FVector& TreeLocation, int32 Depth) -> bool {
        const float Size = LookupTable_VoxelSizeByDepth[Depth] / 2.0f;
        const FCollisionShape Shape = FCollisionShape::MakeBox(FVector(Size));
        
        // Проверяем перекрытие с использованием только что созданной формы
        const bool IsFree = !GetWorld()->OverlapAnyTestByChannel(TreeLocation, FQuat(FRotator(0)), TraceChannel, Shape);
        OctreeRef->IsFree = IsFree;
        OctreeRef->CenterNodeLocation = TreeLocation;
        OctreeRef->Depth = Depth;

    	ProcessGenerateOctrees(OctreeRef);
        
        if (!IsFree && Depth < OctreeDepth)
        {
            const float HalfSize = Size / 2.f;
            if (OctreeRef->Children.IsEmpty())
            {
                OctreeRef->Children.Reserve(8);
                for (int32 Index = 0; Index < 8; ++Index)
                {
                    UOctreeObject* NewChild = NewObject<UOctreeObject>();
                    OctreeRef->Children.Add(NewChild);

                	const uint64 MortonCode = Morton::FVectorToMorton(NewChild->CenterNodeLocation);
                	OctreeRef->ChildrenMortonOctreeArray.Add(MortonCode, NewChild);
                }
            }

            bool HasFreeChildren = false;
            for (uint32 ChildIndex = 0; ChildIndex < 8; ++ChildIndex)
            {
                const FVector ChildLocation = TreeLocation + ChildPositionOffsetByIndex[ChildIndex] * HalfSize;
                HasFreeChildren |= updateOctreeRecursively(OctreeRef->Children[ChildIndex], ChildLocation, Depth + 1);
            }
            if (!HasFreeChildren)
            {
                OctreeRef->Children.Empty();
            }
            return HasFreeChildren;
        }
        else
        {
            OctreeRef->Children.Empty();
            return IsFree;
        }
    };

    // Проверяем и обновляем начальный узел октодерева, начиная с корня
    if (UOctreeObject* OctreeRef = Octrees.IsValidIndex(RootOctreeIndex) ? Octrees[RootOctreeIndex] : nullptr)
    {
        updateOctreeRecursively(OctreeRef, OctreeRef->CenterNodeLocation, 0);
    }
}

