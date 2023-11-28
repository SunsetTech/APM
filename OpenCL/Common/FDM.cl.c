#include "FDM.cl.h"
#include "Math.cl.h"

float FDM_ComputeNextValue(
	int Dimensions,
	const FDM_CellParameters* GridParameters,
	const float* Cells,
	int* Position, const unsigned int* Bounds,
	float SpacetimeDelta
) {
	const int SpacetimeDimensions = Dimensions + 1;
	FDM_CellParameters Parameters = GridParameters[MapIndex(Dimensions, Position+1, Bounds+1)];
	
	float DoubleCurrent = 2.0f * Cells[MapIndex(SpacetimeDimensions,Position, Bounds)];
	Position[0]--;
	float Previous = Cells[MapIndex(SpacetimeDimensions,Position, Bounds)];
	Position[0]++;
	
	float Next = (float)Dimensions * -DoubleCurrent;
	for (unsigned int Dimension = 1; Dimension < SpacetimeDimensions; Dimension++) {
		Position[Dimension]--; 
		Next += Cells[MapIndex(SpacetimeDimensions,Position,Bounds)];
		Position[Dimension]+=2;
		Next += Cells[MapIndex(SpacetimeDimensions,Position,Bounds)];
		Position[Dimension]--;
	}
	
	Next *= SpacetimeDelta * Parameters.WaveVelocity; //TODO get WaveVelocity from parameters
	
	return Clamp(-1.0f, (DoubleCurrent - Previous + Next) * Parameters.TransferEfficiency, 1.0f);
}
