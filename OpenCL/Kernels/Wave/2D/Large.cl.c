#include "Math.cl.h"
#include "FDM.cl.h"

__kernel void FDM_2D_Large(
	const __global FDM_CellParameters* SpacetimeParameters, 
	__global float* Spacetime, unsigned int Timestep,
	__global unsigned int SpacetimeBounds[4]
) {
	const float DT = 0.1f;
	const float DXY = 1.0f;
	const float SpacetimeDelta = pow(DT,2.0f)/pow(DXY,2.0f);
	
	const unsigned int GlobalX = get_global_id(0);
	const unsigned int GlobalY = get_global_id(1);
	const unsigned int LocalX = get_local_id(0);
	const unsigned int LocalY = get_local_id(1);
	
	int PreviousCellPosition[3] = {(int)Timestep-1, GlobalX+LocalX, GlobalY+LocalY};
	
	float Next = FDM_ComputeNextValue(
		2,
		SpacetimeParameters, Spacetime,
		PreviousCellPosition, SpacetimeBounds,
		SpacetimeDelta
	);
	
	int CurrentCellPosition[3] = {Timestep, X, Y};
	Spacetime[MapIndex(3, CurrentCellPosition, SpacetimeBounds)] = Next;
}
