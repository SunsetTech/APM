#include "Wave.cl.h"
#include "Math.cl.h"

Wave_PrecisionType Wave_UpdateND(
	const cl_int SpatialDimensions,
	const Wave_PrecisionType* WaveVelocity,
	const Wave_PrecisionType* TransferEfficiency,
	const Wave_PrecisionType* Spacetime,
	cl_int* Position, 
	const cl_uint* SpacetimeBounds,
	const Wave_PrecisionType SpacetimeDelta
) {
	const int SpacetimeDimensions = SpatialDimensions + 1;
	
	const unsigned int ParameterIndex = MapIndexND(SpatialDimensions, Position+1, SpacetimeBounds+1);

	const Wave_PrecisionType DoubleCurrent = 2.0 * Spacetime[MapIndexND(SpacetimeDimensions,Position, SpacetimeBounds)];
	Position[0]--;
	const Wave_PrecisionType Previous = Spacetime[MapIndexND(SpacetimeDimensions,Position, SpacetimeBounds)];
	Position[0]++;
	
	Wave_PrecisionType Next = (Wave_PrecisionType)SpatialDimensions * -DoubleCurrent;
	for (unsigned int Dimension = 1; Dimension < SpacetimeDimensions; Dimension++) {
		Position[Dimension]--; 
		Next += Spacetime[MapIndexND(SpacetimeDimensions,Position,SpacetimeBounds)];
		Position[Dimension]+=2;
		Next += Spacetime[MapIndexND(SpacetimeDimensions,Position,SpacetimeBounds)];
		Position[Dimension]--;
	}
	
	Next *= SpacetimeDelta * WaveVelocity[ParameterIndex]; //TODO get WaveVelocity from parameters
	
	Next = (DoubleCurrent - Previous + Next) * TransferEfficiency[ParameterIndex]; //TODO parameterized clamping

	return Next;
}

Wave_PrecisionType Wave_Update2D( //Vaguely worth it on the 1650 and slower on the Fury
	const Wave_PrecisionType* WaveVelocity,
	const Wave_PrecisionType* TransferEfficiency,
	const Wave_PrecisionType* Spacetime,
	const          int T,
	const unsigned int StrideT,
	const unsigned int MaxT,
	const          int X,
	const unsigned int StrideX,
	const unsigned int MaxX,
	const          int Y,
	const unsigned int MaxY,
	const Wave_PrecisionType SpacetimeDelta
) {
	
	
	const unsigned int ParameterIndex = MapIndex2D(X, StrideX, Y);

	const Wave_PrecisionType DoubleCurrent = 2.0 * Spacetime[MapIndex3D(T,StrideT,X,StrideX,Y)];
	//T--;
	const Wave_PrecisionType Previous = Spacetime[MapIndex3D(WrapBottom(T-1,MaxT),StrideT,X,StrideX,Y)];
	//T++;
	
	const Wave_PrecisionType Center = -2.0 * DoubleCurrent;
	const Wave_PrecisionType Up = Spacetime[MapIndex3D(T,StrideT,WrapBottom(X-1,MaxX),StrideX,Y)];
	const Wave_PrecisionType Down = Spacetime[MapIndex3D(T,StrideT,WrapTop(X+1,MaxX),StrideX,Y)];
	const Wave_PrecisionType Left = Spacetime[MapIndex3D(T,StrideT,X,StrideX,WrapBottom(Y-1,MaxY))];
	const Wave_PrecisionType Right = Spacetime[MapIndex3D(T,StrideT,X,StrideX,WrapTop(Y+1,MaxY))];
	
	const Wave_PrecisionType Next = (Center+Up+Down+Left+Right) * SpacetimeDelta * WaveVelocity[ParameterIndex]; //TODO get WaveVelocity from parameters
	
	return (DoubleCurrent - Previous + Next) * TransferEfficiency[ParameterIndex]; //TODO parameterized clamping
}
