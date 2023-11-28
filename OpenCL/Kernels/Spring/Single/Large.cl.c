#include "Math.cl.h"
#include "Spring.cl.h"

__kernel void Spring_1D(
	const __global Spring_SpringParameters* SpringParameters, //0
	const __global Spring_NodeParameters* NodeParameters, //1
	__global Spring_NodeState* Spacetime, //2
	const __global unsigned int SpacetimeBounds[2], //3
	unsigned int Timestep, //4
	float TimeDelta //5
) {
	unsigned int MassID = get_global_id(0);
	int ResultCursor[2] = {Timestep, MassID};
	Spacetime[MapIndex(2, ResultCursor, SpacetimeBounds)] = Spring_NextState(
		SpringParameters, 
		NodeParameters, 
		Spacetime, SpacetimeBounds, 
		MassID, 
		Timestep, TimeDelta
	);
}
