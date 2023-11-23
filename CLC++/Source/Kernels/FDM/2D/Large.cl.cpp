#include "Math.cl.hpp"
#include "FDM.cl.hpp"

__kernel void FDM_2D_Large(const __global FDM::CellParameters* GridParameters, __global float* Previous, __global float* Current, unsigned int Width, unsigned int Height) {
	const float DT = 0.1f;
	const float DXY = 1.0f;
	const float SpacetimeDelta = pow(DT,2.0f)/pow(DXY,2.0f);
	unsigned int Dimensions[2] = {Width, Height};
	
	const unsigned int X = get_global_id(0);
	const unsigned int Y = get_global_id(1);
	
	unsigned int Center =   Math::Map::From2DTo1D(X+0, Y+0, Dimensions);
	unsigned int Up =       Math::Map::From2DTo1D(X+0, Y-1, Dimensions);
	unsigned int Down =     Math::Map::From2DTo1D(X+0, Y+1, Dimensions);
	unsigned int Left =     Math::Map::From2DTo1D(X-1, Y+0, Dimensions);
	unsigned int Right =    Math::Map::From2DTo1D(X+1, Y+0, Dimensions);
	
	FDM::CellParameters Parameters = GridParameters[Center];
	float Old = Previous[Center];
	float Older = Current[Center]; //This hasn't been updated since before Previous[Center] was calculated hence is t-2
	float New = 0.0f;
	float DoubleOld = 2.0f * Old;
	New += Previous[Left] - DoubleOld + Previous[Right];
	New += Previous[Down] - DoubleOld + Previous[Up   ];
	New *= SpacetimeDelta * Parameters.WaveVelocity;
	
	Current[Center] = (DoubleOld - Older + New) * Parameters.TransferEfficiency;
}
