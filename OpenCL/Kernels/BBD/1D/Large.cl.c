#include "Math.cl.h"

__kernel void BBD_1D_Large(
	float Input,
	__global float* Spacetime,
	__global unsigned int SpacetimeBounds[2],
	unsigned int Timestep
) {
	const unsigned int SampleIndex = get_global_id(0);
	const int PreviousLeftCursor[] = {Timestep-1, SampleIndex-1};
	const int CurrentCursor[] = {Timestep, SampleIndex};
	
	if (SampleIndex == 0) {
		Spacetime[MapIndex(2,CurrentCursor,SpacetimeBounds)] = Input;
	} else {
		Spacetime[MapIndex(2,CurrentCursor,SpacetimeBounds)] = Spacetime[MapIndex(2,PreviousLeftCursor,SpacetimeBounds)];
	}
}
