#include "Math.cl.h"
#include "Wave.cl.h"

__kernel __attribute__((vec_type_hint(Wave_PrecisionType))) /*__attribute__((reqd_work_group_size(16,16,1)))*/ void Wave_1Dto3D_Large(
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

	const unsigned int MaxT = SpacetimeBounds[0];
	const          int T = Wrap((int)Timestep - 1, MaxT); //Timestep to read from
	const unsigned int MaxX = SpacetimeBounds[1];
	const unsigned int MaxY = SpacetimeBounds[2];
	const unsigned int StrideT = MaxX*MaxY;
	const          int X = get_global_id(0);
	const unsigned int StrideX = MaxY;
	const          int Y = get_global_id(1);
	
	/*const unsigned int TileX = get_group_id(0);
	const unsigned int TileY = get_group_id(1);
	local Wave_PrecisionType Tile[16][16];
	
	event_t CopyEvents[16];
	for (unsigned int Row = 0; Row < 16; Row++) {
		async_work_group_copy(
			Tile[Row], Spacetime+MapIndex3D(T,StrideT,X,StrideX,0), 16,
			CopyEvents[Row]
		);
	}
	wait_group_events(16,CopyEvents);*/

	const Wave_PrecisionType Next = Wave_Update2D(
		WaveVelocity,
		TransferEfficiency,
		Spacetime,
		T, StrideT, MaxT,
		X, StrideX, MaxX,
		Y, MaxY,
		SpacetimeDelta
	);
	
	Spacetime[MapIndex3D(WrapTop(T+1,MaxT),StrideT,X,StrideX,Y)] = Next;
}

__kernel void Wave_1Dto3D_Scatter(
	const          unsigned int                 SpatialDimensions,
	const          unsigned int                 MaxBlockSize,
	const          unsigned int                 SourceIndex,
	const          unsigned int                 TargetIndex, //Timestep to write inputs to
	const          unsigned int                 InputCount,
	const __global unsigned int      * restrict InputPositions,
	const __global Wave_PrecisionType* restrict InputBuffers,
	const __global unsigned int      * restrict SpacetimeBounds,
	      __global Wave_PrecisionType* restrict Spacetime
) {
	const unsigned int InputIndex = get_global_id(0);
	const unsigned int InputPositionsBounds[] = {InputCount, SpatialDimensions};
	               int InputPositionsCursor[] = {InputIndex, 0};
	const unsigned int InputBuffersBounds[] = {InputCount, MaxBlockSize};
	const          int InputBuffersCursor[] = {InputIndex, SourceIndex};
	               int SpacetimeCursor[] = {TargetIndex, 0, 0, 0};
	
	for (unsigned int Dimension = 0; Dimension < SpatialDimensions; Dimension++) {
		InputPositionsCursor[1] = Dimension;
		SpacetimeCursor[Dimension+1] = InputPositions[MapIndexND(2, InputPositionsCursor, InputPositionsBounds)];
	}
	
	Spacetime[MapIndexND(SpatialDimensions+1, SpacetimeCursor, SpacetimeBounds)] = InputBuffers[MapIndexND(2, InputBuffersCursor, InputBuffersBounds)];
}

__kernel void Wave_1Dto3D_Gather(
	const          unsigned int                 SpatialDimensions, //0
	const          unsigned int                 MaxBlockSize, //1
	const          unsigned int                 TargetIndex, //2
	const          unsigned int                 SourceIndex, //3
	const          unsigned int                 OutputCount, //4
	const __global unsigned int      * restrict OutputPositions, //5
	      __global Wave_PrecisionType* restrict OutputBuffers, //6
	const __global unsigned int      * restrict SpacetimeBounds, //7
	const __global Wave_PrecisionType* restrict Spacetime //8
) {
	const unsigned int OutputIndex             =  get_global_id(0)               ;
	const unsigned int OutputPositionsBounds[] = {OutputCount, SpatialDimensions};
	               int OutputPositionsCursor[] = {OutputIndex, 0                };
	const unsigned int OutputBuffersBounds  [] = {OutputCount, MaxBlockSize     };
	const          int OutputBuffersCursor  [] = {OutputIndex, TargetIndex      };
	               int SpacetimeCursor      [] = {SourceIndex, 0          , 0, 0};
	
	for (unsigned int Dimension = 0; Dimension < SpatialDimensions; Dimension++) {
		OutputPositionsCursor[1] = Dimension;
		SpacetimeCursor[Dimension+1] = OutputPositions[MapIndexND(2, OutputPositionsCursor, OutputPositionsBounds)];
	}
	
	OutputBuffers[MapIndexND(2, OutputBuffersCursor, OutputBuffersBounds)] = Spacetime[MapIndexND(SpatialDimensions+1, SpacetimeCursor, SpacetimeBounds)];
}
