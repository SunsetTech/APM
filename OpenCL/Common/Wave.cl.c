#include "Wave.cl.h"
#include "Math.cl.h"

Wave_ValueType Wave_Update(
	const int SpatialDimensions,
	const Wave_CellParameters* GridParameters,
	Wave_ValueType* Spacetime,
	int* Position, const unsigned int* SpacetimeBounds,
	const Wave_ValueType SpacetimeDelta
) {
	const int SpacetimeDimensions = SpatialDimensions + 1;
	const Wave_CellParameters Parameters = GridParameters[MapIndex(SpatialDimensions, Position+1, SpacetimeBounds+1)];
	
	const Wave_ValueType DoubleCurrent = 2.0f * Spacetime[MapIndex(SpacetimeDimensions,Position, SpacetimeBounds)];
	Position[0]--;
	const Wave_ValueType Previous = Spacetime[MapIndex(SpacetimeDimensions,Position, SpacetimeBounds)];
	Position[0]++;
	
	Wave_ValueType Next = (Wave_ValueType)SpatialDimensions * -DoubleCurrent;
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
