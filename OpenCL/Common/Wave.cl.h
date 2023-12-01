#pragma once

typedef struct {
	double WaveVelocity;
	double TransferEfficiency;
} Wave_CellParameters;

double Wave_ComputeNextValue(
	int Dimensions,
	const Wave_CellParameters* GridParameters,
	const double* Cells,
	int* Position, const unsigned int* Bounds,
	double SpacetimeDelta
); 
