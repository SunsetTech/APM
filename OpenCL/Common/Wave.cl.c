#include "Wave.cl.h"
#include "Math.cl.h"

double Wave_ComputeNextValue(
	int Dimensions,
	const Wave_CellParameters* GridParameters,
	const double* Cells,
	int* Position, const unsigned int* Bounds,
	double SpacetimeDelta
) {
	const int SpacetimeDimensions = Dimensions + 1;
	Wave_CellParameters Parameters = GridParameters[MapIndex(Dimensions, Position+1, Bounds+1)];
	
	double DoubleCurrent = 2.0 * Cells[MapIndex(SpacetimeDimensions,Position, Bounds)];
	Position[0]--;
	double Previous = Cells[MapIndex(SpacetimeDimensions,Position, Bounds)];
	Position[0]++;
	
	double Next = (double)Dimensions * -DoubleCurrent;
	for (unsigned int Dimension = 1; Dimension < SpacetimeDimensions; Dimension++) {
		Position[Dimension]--; 
		Next += Cells[MapIndex(SpacetimeDimensions,Position,Bounds)];
		Position[Dimension]+=2;
		Next += Cells[MapIndex(SpacetimeDimensions,Position,Bounds)];
		Position[Dimension]--;
	}
	
	Next *= SpacetimeDelta * 44100.0; //TODO get WaveVelocity from parameters
	
	return (DoubleCurrent - Previous + Next) * Parameters.TransferEfficiency;
}
