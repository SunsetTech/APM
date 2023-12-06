#include "Math.cl.h"
#include "Wave.cl.h"

__kernel void Wave_1Dto3D_Large(
	const __global Wave_CellParameters* SpacetimeParameters, 
	__global Wave_ValueType* Spacetime, 
	unsigned int Timestep,
	unsigned int SpatialDimensions,
	__global unsigned int* SpacetimeBounds
) {
	const Wave_ValueType DT = 1.0f/44100.0f; //TODO make this and the following into parameters
	const Wave_ValueType DXY = 1.0f/10000.0f;
	const Wave_ValueType SpacetimeDelta = pow(DT/DXY,2.0f); //TODO make argument
	
	int CellPosition[4] = {(int)Timestep-1, 0, 0, 0};
	for (unsigned int Dimension = 0; Dimension < SpatialDimensions; Dimension++) {
		CellPosition[Dimension+1] = get_global_id(Dimension);
	}
	
	Wave_Update(
		SpatialDimensions,
		SpacetimeParameters, Spacetime,
		CellPosition, SpacetimeBounds,
		SpacetimeDelta
	);
}
