#include "Math.cl.h"
#include "Wave.cl.h"

__kernel void Wave_1Dto3D_Large(
	const __global Wave_CellParameters* SpacetimeParameters, 
	__global float* Spacetime, 
	unsigned int Timestep,
	unsigned int SpatialDimensions,
	__global unsigned int* SpacetimeBounds
) {
	const float DT = 0.1f;
	const float DXY = 1.0f;
	const float SpacetimeDelta = pow(DT,2.0f)/pow(DXY,2.0f); //TODO make argument
	
	int CellPosition[4] = {(int)Timestep-1, 0, 0, 0};
	for (unsigned int Dimension = 0; Dimension < SpatialDimensions; Dimension++) {
		CellPosition[Dimension+1] = get_global_id(Dimension);
	}
	
	float Next = Wave_ComputeNextValue(
		SpatialDimensions,
		SpacetimeParameters, Spacetime,
		CellPosition, SpacetimeBounds,
		SpacetimeDelta
	);
	
	CellPosition[0] = Timestep;
	Spacetime[MapIndex(SpatialDimensions+1, CellPosition, SpacetimeBounds)] = Next;
}
