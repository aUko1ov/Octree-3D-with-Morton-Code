#pragma once

class FuncLib
{
public:
	static void DrawDebug3DCrosshair(const UWorld* WorldContext, const FVector& Origin,
		float Distance, const FColor& LineColor,
		float Duration = 0.2f, float LineThickness = 1.0f);
};
