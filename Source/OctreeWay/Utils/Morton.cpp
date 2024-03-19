
#include "Morton.h"
#include "CoreMinimal.h"
#include <limits>


uint64 Morton::ExpandBits(uint64 value) {
	value = (value | (value << 32)) & 0x1f00000000ffffULL;
	value = (value | (value << 16)) & 0x1f0000ff0000ffULL;
	value = (value | (value << 8)) & 0x100f00f00f00f00fULL;
	value = (value | (value << 4)) & 0x10c30c30c30c30c3ULL;
	value = (value | (value << 2)) & 0x1249249249249249ULL;
	return value;
}

uint64 Morton::CompactBits(uint64 value) {
	value &= 0x1249249249249249ULL;
	value = (value ^ (value >> 2)) & 0x10c30c30c30c30c3ULL;
	value = (value ^ (value >> 4)) & 0x100f00f00f00f00fULL;
	value = (value ^ (value >> 8)) & 0x1f0000ff0000ffULL;
	value = (value ^ (value >> 16)) & 0x1f00000000ffffULL;
	value = (value ^ (value >> 32)) & 0x1fffff; // Убедимся, что значение остается в пределах 21 бита на ось
	return value;
}

uint64 Morton::FVectorToMorton(FVector Location) {
	// Адаптируем масштабирование, чтобы учесть полный диапазон сцены
	uint64 x = static_cast<uint64>(((Location.X + SceneOffset) / MaxSceneSize) * MaxMorton);
	uint64 y = static_cast<uint64>(((Location.Y + SceneOffset) / MaxSceneSize) * MaxMorton);
	uint64 z = static_cast<uint64>(((Location.Z + SceneOffset) / MaxSceneSize) * MaxMorton);

	// Расширяем биты и комбинируем
	x = ExpandBits(x);
	y = ExpandBits(y);
	z = ExpandBits(z);

	return x | (y << 1) | (z << 2);
}

FVector Morton::MortonToFVector(uint64 MortonCode) {
	// Сжимаем биты и масштабируем обратно в координаты
	uint64 x = CompactBits(MortonCode);
	uint64 y = CompactBits(MortonCode >> 1);
	uint64 z = CompactBits(MortonCode >> 2);

	FVector Location;
	Location.X = (static_cast<float>(x) / MaxMorton) * MaxSceneSize - SceneOffset;
	Location.Y = (static_cast<float>(y) / MaxMorton) * MaxSceneSize - SceneOffset;
	Location.Z = (static_cast<float>(z) / MaxMorton) * MaxSceneSize - SceneOffset;

	return Location;
}