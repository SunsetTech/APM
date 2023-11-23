#include "Math.cl.hpp"
#include "FDM.cl.hpp"

__kernel void FDM_2D_Buffered_SOC(const __global FDM::CellParameters* GridParameters, __global float* Buffer, float SpacetimeDelta, float WaveVelocitySquared, unsigned int GroupWidth, unsigned int GroupHeight, unsigned int Iterations, unsigned int Width, unsigned int Height) {
	unsigned int WorkDimensions[5] = {GroupWidth, GroupHeight, Iterations, Width, Height};
	unsigned int ParameterDimensions[4] = {GroupWidth, GroupHeight, Width, Height};
    int GroupX = get_group_id(0);
	int GroupY = get_group_id(1);
	int LocalX = get_local_id(0);
    int LocalY = get_local_id(1);

	for (int Iteration = 0; Iteration < WorkDimensions[2]; Iteration++) {
		barrier(CLK_GLOBAL_MEM_FENCE);
		FDM::CellParameters Parameters = GridParameters[Math::Map::From4DTo1D(GroupX, GroupY, LocalX, LocalY, ParameterDimensions)];

		unsigned int Current =  Math::Map::From5DTo1D(GroupX, GroupY, Iteration-0, LocalX+0, LocalY+0, WorkDimensions);
		unsigned int Prev =     Math::Map::From5DTo1D(GroupX, GroupY, Iteration-1, LocalX+0, LocalY+0, WorkDimensions);
		unsigned int PrevPrev = Math::Map::From5DTo1D(GroupX, GroupY, Iteration-2, LocalX+0, LocalY+0, WorkDimensions);
		unsigned int Up =       Math::Map::From5DTo1D(GroupX, GroupY, Iteration-1, LocalX+0, LocalY-1, WorkDimensions);
		unsigned int Down =     Math::Map::From5DTo1D(GroupX, GroupY, Iteration-1, LocalX+0, LocalY+1, WorkDimensions);
		unsigned int Left =     Math::Map::From5DTo1D(GroupX, GroupY, Iteration-1, LocalX-1, LocalY+0, WorkDimensions);
		unsigned int Right =    Math::Map::From5DTo1D(GroupX, GroupY, Iteration-1, LocalX+1, LocalY+0, WorkDimensions);
		
		float Old = Buffer[Prev];
		float Older = Buffer[PrevPrev];
		float New = 0.0f;
		float DoubleOld = 2.0f * Old;
		New += Buffer[Left] - DoubleOld + Buffer[Right];
		New += Buffer[Down] - DoubleOld + Buffer[Up   ];
		New *= SpacetimeDelta * Parameters.WaveVelocity;
		
		Buffer[Current] = (DoubleOld - Older + New) * Parameters.TransferEfficiency;
	}
}
