#include "Math.cl.h"
#include "FDM.cl.h"

__kernel void FDM_4D_Large(
	const __global FDM_CellParameters* SpacetimeParameters, 
	__global float* Spacetime, unsigned int Timestep,
	__global unsigned int SpacetimeBounds[5]
) {
	unsigned int P_SpacetimeBounds[5];
	for (unsigned int Dimension = 0; Dimension < 6; Dimension++) {\
		P_SpacetimeBounds[Dimension] = SpacetimeBounds[Dimension];
	}
	const float DT = 0.1f;
	const float DXY = 1.0f;
	const float SpacetimeDelta = pow(DT,2.0f)/pow(DXY,2.0f);
	
	int CellPosition[5] = {(int)Timestep-1, 0, 0, 0, 0};
	for (unsigned int Dimension = 0; Dimension < 3; Dimension++) {
		CellPosition[Dimension+1] = get_global_id(Dimension) + get_local_id(Dimension);
	}
	
	for (int Z = 0; Z < SpacetimeBounds[4]; Z++) {
		CellPosition[0] = Timestep - 1;
		CellPosition[4] = Z;
		float Next = FDM_ComputeNextValue(
			4,
			SpacetimeParameters, Spacetime,
			CellPosition, P_SpacetimeBounds,
			SpacetimeDelta
		);
		
		CellPosition[0] = Timestep;
		Spacetime[MapIndex(5, CellPosition, P_SpacetimeBounds)] = Next;
	}
}
