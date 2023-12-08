#include "Wave.cl.h"
#include "Math.cl.h"

Wave_PrecisionType Wave_Update(
	const cl_int SpatialDimensions,
	const Wave_CellParameters* GridParameters,
	Wave_PrecisionType* Spacetime,
	cl_int* Position, const cl_uint* SpacetimeBounds,
	const Wave_PrecisionType SpacetimeDelta
) {
	const cl_int SpacetimeDimensions = SpatialDimensions + 1;
	const Wave_CellParameters Parameters = GridParameters[MapIndex(SpatialDimensions, Position+1, SpacetimeBounds+1)];
	
	const Wave_PrecisionType DoubleCurrent = 2.0f * Spacetime[MapIndex(SpacetimeDimensions,Position, SpacetimeBounds)];
	Position[0]--;
	const Wave_PrecisionType Previous = Spacetime[MapIndex(SpacetimeDimensions,Position, SpacetimeBounds)];
	Position[0]++;
	
	Wave_PrecisionType Next = (Wave_PrecisionType)SpatialDimensions * -DoubleCurrent;
	for (unsigned int Dimension = 1; Dimension < SpacetimeDimensions; Dimension++) {
		Position[Dimension]--; 
		Next += Spacetime[MapIndex(SpacetimeDimensions,Position,SpacetimeBounds)];
		Position[Dimension]+=2;
		Next += Spacetime[MapIndex(SpacetimeDimensions,Position,SpacetimeBounds)];
		Position[Dimension]--;
	}
	
	Next *= SpacetimeDelta * Parameters.WaveVelocity; //TODO get WaveVelocity from parameters
	
	Next = (DoubleCurrent - Previous + Next) * Parameters.TransferEfficiency; //TODO parameterized clamping

	return Next;
}
