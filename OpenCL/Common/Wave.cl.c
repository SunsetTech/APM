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

Wave_PrecisionType Wave_Update2D(
	const Wave_PrecisionType* WaveVelocity,
	const Wave_PrecisionType* TransferEfficiency,
	const Wave_PrecisionType* Spacetime,
	cl_int* Position, 
	const cl_uint* SpacetimeBounds,
	const Wave_PrecisionType SpacetimeDelta
) {
	
	const unsigned int MaxT = SpacetimeBounds[0];
	const          int T = Wrap(Position[0], MaxT);
	const unsigned int StrideT = SpacetimeBounds[1]*SpacetimeBounds[2];
	const          int X = Position[1];
	const unsigned int MaxX = SpacetimeBounds[1];
	const unsigned int StrideX = SpacetimeBounds[2];
	const          int Y = Position[2];
	const unsigned int MaxY = SpacetimeBounds[2];
	
	const unsigned int ParameterIndex = MapIndexND(2, Position+1, SpacetimeBounds+1);

	const Wave_PrecisionType DoubleCurrent = 2.0 * Spacetime[MapIndex3D(T,StrideT,X,StrideX,Y)];
	//T--;
	const Wave_PrecisionType Previous = Spacetime[MapIndex3D(Wrap(T-1,MaxT),StrideT,X,StrideX,Y)];
	//T++;
	
	Wave_PrecisionType Next = (Wave_PrecisionType)2 * -DoubleCurrent;
	Next += Spacetime[MapIndex3D(T,StrideT,Wrap(X-1,MaxX),StrideX,Y)];
	Next += Spacetime[MapIndex3D(T,StrideT,Wrap(X+1,MaxX),StrideX,Y)];
	Next += Spacetime[MapIndex3D(T,StrideT,X,StrideX,Wrap(Y-1,MaxY))];
	Next += Spacetime[MapIndex3D(T,StrideT,X,StrideX,Wrap(Y+1,MaxY))];
	
	Next *= SpacetimeDelta * WaveVelocity[ParameterIndex]; //TODO get WaveVelocity from parameters
	
	Next = (DoubleCurrent - Previous + Next) * TransferEfficiency[ParameterIndex]; //TODO parameterized clamping

	return Next;
}
