#include "Math.cl.h"
#include "Spring.cl.h"

__kernel void Spring_Bundle_Large( //Bundle of disconnected springs
	const __global Spring_SpringParameters* SpringParameters, //0
	const __global Spring_NodeParameters* NodeParameters, //1
	unsigned int GroupBounds, //2
	__global Spring_NodeState* Spacetime, //3
	const __global unsigned int SpacetimeBounds[2], //4
	unsigned int Timestep, //5
	float TimeDelta //6
) {
	const unsigned int BundleBounds[] = {GroupBounds, SpacetimeBounds[0], SpacetimeBounds[1]};
	const unsigned int FiberID = get_global_id(0);
	const unsigned int MassID = get_global_id(1);
	int GroupGursor[] = {FiberID, 0, 0};
	int ResultCursor[3] = {FiberID, Timestep, MassID};
	//printf("{%i, %i, %i}\n", BundleBounds[0], BundleBounds[1], BundleBounds[2]);
	Spring_NodeState NewState = Spring_NextState(
		SpringParameters, 
		NodeParameters, 
		Spacetime + MapIndex(3, GroupGursor, BundleBounds), SpacetimeBounds, 
		MassID,
		Timestep, TimeDelta
	);
	unsigned int ResultIndex = MapIndex(3, ResultCursor, BundleBounds);
	//printf("(%i, %i, %i)=%i -> {%f, %f}\n", ResultCursor[0], ResultCursor[1], ResultCursor[2], ResultIndex, NewState.Position, NewState.Velocity);
	Spacetime[ResultIndex].Position = NewState.Position;
	Spacetime[ResultIndex].Velocity = NewState.Velocity;
}
