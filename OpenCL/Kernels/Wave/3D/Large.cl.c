#include "Math.cl.h"
#include "FDM.cl.h"

__kernel void FDM_3D_Large(
	const __global FDM_CellParameters* SpacetimeParameters, 
	__global float* Spacetime, 
	unsigned int Timestep,
	__global unsigned int SpacetimeBounds[4]
) {
	const float DT = 0.1f;
	const float DXY = 1.0f;
	const float SpacetimeDelta = pow(DT,2.0f)/pow(DXY,2.0f);
	
	int CellPosition[4] = {(int)Timestep-1, 0, 0, 0};
	for (unsigned int Dimension = 0; Dimension < 3; Dimension++) {
		CellPosition[Dimension+1] = get_global_id(Dimension);
		//printf("%i:(%i+%i)=%i\n",Dimension+1,get_global_id(Dimension),get_local_id(Dimension),CellPosition[Dimension+1]);
	}
	//printf("%i %i %i\n", CellPosition[1], CellPosition[2], CellPosition[3]);
	float Next = FDM_ComputeNextValue(
		3,
		SpacetimeParameters, Spacetime,
		CellPosition, SpacetimeBounds,
		SpacetimeDelta
	);
	
	CellPosition[0] = Timestep;
	Spacetime[MapIndex(4, CellPosition, SpacetimeBounds)] = Next;
}
