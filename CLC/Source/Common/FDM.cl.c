#include "FDM.cl.h"

float FDM_ComputeNextValue(
	int Dimensions,
	const __global FDM_CellParameters* GridParameters, unsigned int ParametersRegionStart,
	const __global float* Cells, unsigned int CellsRegionStart,
	int* Position, const unsigned int* Bounds,
	float SpacetimeDelta
) {
	const int SpacetimeDimensions = Dimensions + 1;
	FDM_CellParameters Parameters = GridParameters[ParametersRegionStart + MapIndex(Dimensions, Position+1, Bounds+1)];
	
	float DoubleCurrent = 2.0f * Cells[CellsRegionStart + MapIndex(SpacetimeDimensions,Position, Bounds)];
	Position[0]--;
	float Previous = Cells[CellsRegionStart + MapIndex(SpacetimeDimensions,Position, Bounds)];
	Position[0]++;
	
	float Next = (float)Dimensions * -DoubleCurrent;
	for (unsigned int Dimension = 1; Dimension < SpacetimeDimensions; Dimension++) {
		Position[Dimension]--; 
		Next += Cells[CellsRegionStart + MapIndex(SpacetimeDimensions,Position,Bounds)];
		Position[Dimension]+=2;
		Next += Cells[CellsRegionStart + MapIndex(SpacetimeDimensions,Position,Bounds)];
		Position[Dimension]--;
	}
	
	Next *= SpacetimeDelta * Parameters.WaveVelocity; //TODO get WaveVelocity from parameters
	
	return Clamp(-1.0f, (DoubleCurrent - Previous + Next) * Parameters.TransferEfficiency, 1.0f);
}
