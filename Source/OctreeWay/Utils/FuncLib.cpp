#include "FuncLib.h"

void FuncLib::DrawDebug3DCrosshair(
	const UWorld* WorldContext, const FVector& Origin, float Distance,
	const FColor& LineColor, float Duration, float LineThickness)
{
	if (!WorldContext)
	{
		return;
	}

	// Определение конечных точек для линий
	FVector LineEnds[4] = {
		Origin + FVector(Distance, 0.f, 0.f),  // вправо
		Origin - FVector(Distance, 0.f, 0.f),  // влево
		Origin + FVector(0.f, Distance, 0.f),  // вперед
		Origin - FVector(0.f, Distance, 0.f)   // назад
	};

	// Рисование линий
	for (const FVector& End : LineEnds)
	{
		DrawDebugLine(
			WorldContext,
			Origin,
			End,
			LineColor,
			false,          // bPersistentLines
			Duration,       // Время отображения
			0,              // Индекс глубины
			LineThickness   // Толщина линии
		);
	}
}