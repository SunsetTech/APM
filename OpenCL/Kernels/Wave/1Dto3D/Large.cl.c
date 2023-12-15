#include "Math.cl.h"
#include "Wave.cl.h"

__kernel __attribute__((vec_type_hint(Wave_PrecisionType))) void Wave_1Dto3D_Large(
	const          unsigned int                 SpatialDimensions,
	const          unsigned int                 Timestep, //Which timestep we write results to
	const          Wave_PrecisionType           TimeDelta,
	const          Wave_PrecisionType           SpaceDelta,
	const __global unsigned int      * restrict SpacetimeBounds,
	const __global Wave_PrecisionType* restrict WaveVelocity,
	const __global Wave_PrecisionType* restrict TransferEfficiency,
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

__kernel void Wave_1Dto3D_Scatter(
	const          unsigned int                 SpatialDimensions,
	const          unsigned int                 BlockSize,
	const          unsigned int                 SubIteration,
	const          unsigned int                 Timestep, //Timestep to write inputs to
	const          unsigned int                 InputCount,
	const __global unsigned int      * restrict InputPositions,
	const __global Wave_PrecisionType* restrict InputBuffers,
	const __global unsigned int      * restrict SpacetimeBounds,
	      __global Wave_PrecisionType* restrict Spacetime
) {
	const unsigned int InputIndex = get_global_id(0);
	const unsigned int InputPositionsBounds[] = {InputCount, SpatialDimensions};
	               int InputPositionsCursor[] = {InputIndex, 0};
	const unsigned int InputBuffersBounds[] = {InputCount, BlockSize};
	const          int InputBuffersCursor[] = {InputCount, SubIteration};
	               int SpacetimeCursor[] = {Timestep, 0, 0, 0};
	
	for (unsigned int Dimension = 0; Dimension < SpatialDimensions; Dimension++) {
		InputPositionsCursor[1] = Dimension;
		SpacetimeCursor[Dimension+1] = InputPositions[MapIndex(2, InputPositionsCursor, InputPositionsBounds)];
	}
	
	Spacetime[MapIndex(SpatialDimensions+1, SpacetimeCursor, SpacetimeBounds)] = InputBuffers[MapIndex(2, InputBuffersCursor, InputBuffersBounds)];
}

__kernel void Wave_1Dto3D_Gather(
	const          unsigned int                 SpatialDimensions,
	const          unsigned int                 BlockSize,
	const          unsigned int                 SubIteration,
	const          unsigned int                 Timestep,
	const          unsigned int                 OutputCount,
	const __global unsigned int      * restrict OutputPositions,
	      __global Wave_PrecisionType* restrict OutputBuffers,
	const __global unsigned int      * restrict SpacetimeBounds,
	const __global Wave_PrecisionType* restrict Spacetime
) {
	const unsigned int OutputIndex             =  get_global_id(0)               ;
	const unsigned int OutputPositionsBounds[] = {OutputCount, SpatialDimensions};
	               int OutputPositionsCursor[] = {OutputIndex, 0                };
	const unsigned int OutputBuffersBounds  [] = {OutputCount, BlockSize        };
	const          int OutputBuffersCursor  [] = {OutputCount, SubIteration     };
	               int SpacetimeCursor      [] = {Timestep,    0,           0, 0};
	
	for (unsigned int Dimension = 0; Dimension < SpatialDimensions; Dimension++) {
		OutputPositionsCursor[1] = Dimension;
		SpacetimeCursor[Dimension+1] = OutputPositions[MapIndex(2, OutputPositionsCursor, OutputPositionsBounds)];
	}
	
	OutputBuffers[MapIndex(2, OutputBuffersCursor, OutputBuffersBounds)] = Spacetime[MapIndex(SpatialDimensions+1, SpacetimeCursor, SpacetimeBounds)];
}

__kernel void Wave_1Dto3D_Chunked(
	const          unsigned int                 SpatialDimensions,
	const          unsigned int                 StartIteration,
	const          unsigned int                 BlockSize,
	const          unsigned int                 InputCount,
	const          unsigned int                 OutputCount,
	const          Wave_PrecisionType           TimeDelta,
	const          Wave_PrecisionType           SpaceDelta,
	const __global unsigned int      * restrict SpacetimeBounds,
	const __global unsigned int      * restrict InputPositions,
	const __global unsigned int      * restrict OutputPositions,
	const __global Wave_PrecisionType* restrict InputBuffers,
	const __global Wave_PrecisionType* restrict WaveVelocity,
	const __global Wave_PrecisionType* restrict TransferEfficiency,
	      __global Wave_PrecisionType* restrict Spacetime,
	      __global Wave_PrecisionType* restrict OutputBuffers
) {
	const ndrange_t ScatterRange = ndrange_1D(InputCount);
	const ndrange_t GatherRange = ndrange_1D(OutputCount);
	ndrange_t WorkRange;
	WorkRange.workDimension = SpatialDimensions;
	for (unsigned int Dimension = 0; Dimension < SpatialDimensions; Dimension++) {
		WorkRange.globalWorkSize[Dimension] = SpacetimeBounds[Dimension+1];
	}
	
	clk_event_t StartEvent = create_user_event(); 
	set_user_event_status(StartEvent, CL_COMPLETE);
	clk_event_t ScatterEvent, ExecutionEvent, GatherEvent;
	queue_t Queue = get_default_queue();
	for (unsigned int SubIteration = 0; SubIteration < BlockSize; SubIteration++) {
		unsigned int Iteration = StartIteration+SubIteration;
		unsigned int PreviousTimestep = Iteration%2;
		unsigned int CurrentTimestep = (Iteration+1)%2;
		enqueue_kernel( //TODO interleave with last iterations gather operation
			Queue,
			CLK_ENQUEUE_FLAGS_NO_WAIT,
			ScatterRange,
			1, &StartEvent, &ScatterEvent,
			^{
				Wave_1Dto3D_Scatter(
					SpatialDimensions,
					BlockSize,
					SubIteration,
					PreviousTimestep,
					InputCount,
					InputPositions,
					InputBuffers,
					SpacetimeBounds,
					Spacetime
				);
			}
		); release_event(StartEvent);
		enqueue_kernel(
			Queue,
			CLK_ENQUEUE_FLAGS_NO_WAIT,
			WorkRange,
			1, &ScatterEvent, &ExecutionEvent,
			^{
				Wave_1Dto3D_Large(
					SpatialDimensions,
					CurrentTimestep,
					TimeDelta, SpaceDelta,
					SpacetimeBounds,
					WaveVelocity, TransferEfficiency,
					Spacetime
				);
			}
		); release_event(ScatterEvent);
		enqueue_kernel(
			Queue,
			CLK_ENQUEUE_FLAGS_NO_WAIT,
			GatherRange,
			1, &ExecutionEvent, &GatherEvent,
			^{
				Wave_1Dto3D_Gather(
					SpatialDimensions,
					BlockSize,
					SubIteration,
					CurrentTimestep,
					OutputCount,
					OutputPositions,
					OutputBuffers,
					SpacetimeBounds,
					Spacetime
				);
			}
		); release_event(ExecutionEvent);
		StartEvent = GatherEvent;
	}
	release_event(StartEvent);
}
