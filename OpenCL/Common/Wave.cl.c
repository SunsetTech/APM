#include "Wave.cl.h"
#include "Math.cl.h"

Wave_PrecisionType Wave_Update(
	const cl_int SpatialDimensions,
	const Wave_PrecisionType* WaveVelocity,
	const Wave_PrecisionType* TransferEfficiency,
	const Wave_PrecisionType* Spacetime,
	cl_int* Position, 
	const cl_uint* SpacetimeBounds,
	local const cl_uint* ParameterIndexMultipliers,
	local const cl_uint* SpacetimeIndexMultipliers,
	const Wave_PrecisionType SpacetimeDelta
) {
	const int SpacetimeDimensions = SpatialDimensions + 1;
	
	const unsigned int ParameterIndex = MapIndex(SpatialDimensions, Position+1, SpacetimeBounds+1, ParameterIndexMultipliers);

	const Wave_PrecisionType DoubleCurrent = 2.0 * Spacetime[MapIndex(SpacetimeDimensions,Position, SpacetimeBounds, SpacetimeIndexMultipliers)];
	Position[0]--;
	const Wave_PrecisionType Previous = Spacetime[MapIndex(SpacetimeDimensions,Position, SpacetimeBounds, SpacetimeIndexMultipliers)];
	Position[0]++;
	
	Wave_PrecisionType Next = (Wave_PrecisionType)SpatialDimensions * -DoubleCurrent;
	for (unsigned int Dimension = 1; Dimension < SpacetimeDimensions; Dimension++) {
		Position[Dimension]--; 
		Next += Spacetime[MapIndex(SpacetimeDimensions,Position,SpacetimeBounds,SpacetimeIndexMultipliers)];
		Position[Dimension]+=2;
		Next += Spacetime[MapIndex(SpacetimeDimensions,Position,SpacetimeBounds,SpacetimeIndexMultipliers)];
		Position[Dimension]--;
	}
	
	Next *= SpacetimeDelta * WaveVelocity[ParameterIndex]; //TODO get WaveVelocity from parameters
	
	Next = (DoubleCurrent - Previous + Next) * TransferEfficiency[ParameterIndex]; //TODO parameterized clamping

	return Next;
}
