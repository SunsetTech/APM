#include "Math.cl.h"
#include "Wave.cl.h"

__kernel void Wave_1Dto3D_Large(
	const unsigned int SpatialDimensions,
	const __global unsigned int* SpacetimeBounds,
	const __global Wave_CellParameters* SpacetimeParameters, 
	const Wave_PrecisionType SpaceDelta,
	const Wave_PrecisionType TimeDelta,
	const unsigned int Timestep,
	__global Wave_PrecisionType* Spacetime 
) {
	const Wave_PrecisionType SpacetimeDelta = pow(TimeDelta/SpaceDelta,2.0f); //TODO make argument
	
	int CellPosition[4] = {
		(int)Timestep-1, 
		get_global_id(0), 
		get_global_id(1), 
		get_global_id(2),
	};
	
	const Wave_PrecisionType Next = Wave_Update(
		SpatialDimensions,
		SpacetimeParameters, Spacetime,
		CellPosition, SpacetimeBounds,
		SpacetimeDelta
	);
	
	CellPosition[0]++;
	Spacetime[MapIndex(SpatialDimensions+1, CellPosition, SpacetimeBounds)] = Next;
}
