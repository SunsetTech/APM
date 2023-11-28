#include "Math.cl.h" //TODO add the root folder as include-directory
#include "FDM.cl.h"

__kernel void FDM_2D_Buffered_SOG(
	const __global FDM_CellParameters* GridParameters, __global float* Grid, 
	unsigned int GroupWidth, unsigned int GroupHeight, 
	unsigned int CellsPerGroupX, unsigned int CellsPerGroupY, 
	unsigned int CellsPerWorkX, unsigned int CellsPerWorkY,
	unsigned int IterationsOnDevice,
	__global float* LeftChannel, __global float* RightChannel
) {
	const float DT = 0.1f;
	const float DS = 1.0f;
	const float SpacetimeDelta = pow(DT/DS,2.0f);
	
	unsigned int GridWidth = CellsPerGroupX * CellsPerWorkX;
	unsigned int GridHeight = CellsPerGroupY * CellsPerWorkY;
	
	unsigned int WorkBounds[5] = {GroupWidth, GroupHeight, 2, GridWidth, GridHeight};
	unsigned int ParameterBounds[4] = {GroupWidth, GroupHeight, GridWidth, GridHeight};
	unsigned int ChannelBounds[3] = {GroupWidth, GroupHeight, IterationsOnDevice};
	unsigned int SpacetimeBounds[3] = {2, GridWidth, GridHeight};

	unsigned int GroupX = get_group_id(0);
	unsigned int GroupY = get_group_id(1);
	unsigned int LocalX = get_local_id(0);
    unsigned int LocalY = get_local_id(1);
	
	int GroupSpacetimeStartPosition[5] = {(int)GroupX, (int)GroupY, 0, 0, 0};
	unsigned int GroupSpacetimeStart = MapIndex(5, GroupSpacetimeStartPosition, WorkBounds);
	
	int GroupParametersStartPosition[4] = {(int)GroupX, (int)GroupY, 0, 0};
	unsigned int GroupParametersStart = MapIndex(4, GroupParametersStartPosition, ParameterBounds);
	
	int LocalCellPosition[3];
	int CurrentCellPosition[5];
	int ChannelPosition[3];
	for (unsigned int Iteration = 0; Iteration < IterationsOnDevice; Iteration++) {
		barrier(CLK_GLOBAL_MEM_FENCE);
		for (unsigned int CellX = 0; CellX < CellsPerWorkX; CellX++) {
			for (unsigned int CellY = 0; CellY < CellsPerWorkY; CellY++) {
				unsigned int X = LocalX * CellsPerWorkX + CellX;
				unsigned int Y = LocalY * CellsPerWorkY + CellY;
				
				//LocalCellPosition = {((int)Iteration)-1, (int)X, (int)Y};
				LocalCellPosition[0] = ((int)Iteration)-1;
				LocalCellPosition[1] = X;
				LocalCellPosition[2] = Y;
				float Next = FDM_ComputeNextValue(
					2,
					GridParameters + GroupParametersStart, 
					Grid + GroupSpacetimeStart,
					LocalCellPosition, (const unsigned int*)SpacetimeBounds,
					SpacetimeDelta
				);
				
				//int CurrentCellPosition[5] = {(int)GroupX, (int)GroupY, (int)Iteration, (int)X, (int)Y};
				CurrentCellPosition[0] = GroupX;
				CurrentCellPosition[1] = GroupY;
				CurrentCellPosition[2] = Iteration;
				CurrentCellPosition[3] = X;
				CurrentCellPosition[4] = Y;
				unsigned int IndexCurrent =  MapIndex(5, CurrentCellPosition, WorkBounds);
				Grid[IndexCurrent] = Next;
				
				//int ChannelPosition[3] = {(int)GroupX, (int)GroupY, (int)Iteration};
				ChannelPosition[0] = GroupX;
				ChannelPosition[1] = GroupY;
				ChannelPosition[2] = Iteration;
				unsigned int ChannelIndex = MapIndex(3, ChannelPosition, ChannelBounds);
				if (X == 1 && Y == 1) {
					LeftChannel[ChannelIndex] = Next;
				}
				if (X == GridWidth-2 && Y == GridHeight-2) {
					RightChannel[ChannelIndex] = Next;
				}
			}
		}
	}
}
