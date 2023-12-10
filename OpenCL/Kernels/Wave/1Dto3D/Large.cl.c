#include "Math.cl.h"
#include "Wave.cl.h"

__kernel __attribute__((vec_type_hint(Wave_PrecisionType))) void Wave_1Dto3D_Large(
	const unsigned int SpatialDimensions,
	const __global unsigned int* restrict SpacetimeBounds,
	const __global Wave_PrecisionType* restrict WaveVelocity,
	const __global Wave_PrecisionType* restrict TransferEfficiency,
	const Wave_PrecisionType SpaceDelta,
	const Wave_PrecisionType TimeDelta,
	const unsigned int Timestep,
	__global Wave_PrecisionType* restrict Spacetime 
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
		WaveVelocity,
		TransferEfficiency,
		Spacetime,
		CellPosition, SpacetimeBounds,
		SpacetimeDelta
	);
	
	CellPosition[0]++;
	Spacetime[MapIndex(SpatialDimensions+1, CellPosition, SpacetimeBounds)] = Next;
}
