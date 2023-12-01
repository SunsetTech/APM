#include "Math.cl.h"
#include "Wave.cl.h"

__kernel void Wave_1Dto3D_Large(
	const __global Wave_CellParameters* SpacetimeParameters, 
	__global double* Spacetime, 
	unsigned int Timestep,
	unsigned int SpatialDimensions,
	__global unsigned int* SpacetimeBounds
) {
	const double DT = 1.0f/44100.0f;
	const double DXY = 1.0f;
	const double SpacetimeDelta = pow(DT/DXY,2.0); //TODO make argument
	
	int CellPosition[4] = {(int)Timestep-1, 0, 0, 0};
	for (unsigned int Dimension = 0; Dimension < SpatialDimensions; Dimension++) {
		CellPosition[Dimension+1] = get_global_id(Dimension);
	}
	
	double Next = Wave_ComputeNextValue(
		SpatialDimensions,
		SpacetimeParameters, Spacetime,
		CellPosition, SpacetimeBounds,
		SpacetimeDelta
	);
	
	CellPosition[0] = Timestep;
	Spacetime[MapIndex(SpatialDimensions+1, CellPosition, SpacetimeBounds)] = Next;
}
