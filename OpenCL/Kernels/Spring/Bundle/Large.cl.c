#include "Math.cl.h"
#include "Spring.cl.h"

__kernel __attribute__((vec_type_hint(Spring_PrecisionType))) void Spring_Bundle_Large( //Bundle of disconnected springs
	               unsigned int                       Timestep        ,
	               Spring_PrecisionType               TimeDelta       ,
	const __global unsigned int            * restrict BundleBounds    ,
	const __global Spring_SpringParameters * restrict SpringParameters,
	const __global Spring_NodeParameters   * restrict NodeParameters  ,
	      __global Spring_NodeState        * restrict Spacetime
) {
	const unsigned int *SpacetimeBounds = BundleBounds+1;
	const unsigned int FiberID = get_global_id(0);
	const unsigned int MassID = get_global_id(1);
	int GroupCursor[] = {FiberID, 0, 0};
	int ResultCursor[3] = {FiberID, Timestep, MassID};
	Spring_NodeState NewState = Spring_NextState(
		SpringParameters, 
		NodeParameters, 
		Spacetime + MapIndex(3, GroupCursor, BundleBounds), SpacetimeBounds, 
		MassID,
		Timestep, TimeDelta
	);
	unsigned int ResultIndex = MapIndex(3, ResultCursor, BundleBounds);
	Spacetime[ResultIndex].Position = NewState.Position;
	Spacetime[ResultIndex].Velocity = NewState.Velocity;
}

__kernel void Spring_Bundle_Scatter(
	const          unsigned int                   MaxBlockSize,
	const          unsigned int                   SourceIndex,
	const          unsigned int                   TargetIndex, //Timestep to write inputs to
	const          unsigned int                   InputCount,
	const __global unsigned int        * restrict InputPositions,
	const __global Spring_PrecisionType* restrict InputBuffers,
	const __global unsigned int        * restrict BundleBounds,
	      __global Spring_NodeState    * restrict Spacetime
) {
	const unsigned int InputIndex = get_global_id(0);
	const unsigned int InputPositionsBounds[] = {InputCount, 2};
	               int InputPositionsCursor[] = {InputIndex, 0};
	const unsigned int InputBuffersBounds[] = {InputCount, MaxBlockSize};
	const          int InputBuffersCursor[] = {InputIndex, SourceIndex};
	               int SpacetimeCursor[] = {0, TargetIndex, 0};
	
	SpacetimeCursor[0] = InputPositions[MapIndex(2, InputPositionsCursor, InputPositionsBounds)];
	InputPositionsCursor[1] = 1;
	SpacetimeCursor[2] = InputPositions[MapIndex(2, InputPositionsCursor, InputPositionsBounds)];
	
	Spacetime[MapIndex(3, SpacetimeCursor, BundleBounds)].Position = InputBuffers[MapIndex(2, InputBuffersCursor, InputBuffersBounds)];
}

__kernel void Wave_1Dto3D_Gather(
	const          unsigned int                   MaxBlockSize,
	const          unsigned int                   TargetIndex,
	const          unsigned int                   SourceIndex,
	const          unsigned int                   OutputCount,
	const __global unsigned int        * restrict OutputPositions,
	      __global Spring_PrecisionType* restrict OutputBuffers,
	const __global unsigned int        * restrict BundleBounds,
	const __global Spring_NodeState    * restrict Spacetime
) {
	const unsigned int OutputIndex = get_global_id(0);
	const unsigned int OutputPositionsBounds[] = {OutputCount, 2};
	               int OutputPositionsCursor[] = {OutputIndex, 0};
	const unsigned int OutputBuffersBounds[] = {OutputCount, MaxBlockSize};
	const          int OutputBuffersCursor[] = {OutputIndex, SourceIndex};
	               int SpacetimeCursor[] = {0, TargetIndex, 0};
	
	SpacetimeCursor[0] = OutputPositions[MapIndex(2, OutputPositionsCursor, OutputPositionsBounds)];
	OutputPositionsCursor[1] = 1;
	SpacetimeCursor[2] = OutputPositions[MapIndex(2, OutputPositionsCursor, OutputPositionsBounds)];
	
	OutputBuffers[MapIndex(2, OutputBuffersCursor, OutputBuffersBounds)] = Spacetime[MapIndex(3, SpacetimeCursor, BundleBounds)].Position;
}

