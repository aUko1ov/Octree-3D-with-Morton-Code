
#pragma once

#include "CoreMinimal.h"


class Morton
{
public:
	static uint64 FVectorToMorton(FVector Location);
	static FVector MortonToFVector(uint64 MortonCode);

private:
	static uint64 CompactBits(uint64 value);
	static uint64 ExpandBits(uint64 value);

	constexpr static float MaxSceneSize = 100000.0f; // Общий размер сцены в одном направлении
	constexpr static float SceneOffset = MaxSceneSize / 2.0f; // Смещение для перевода координат в положительный диапазон
	constexpr static uint64 MaxMorton = (1ULL << 21) - 1; // Максимум для одной оси
};