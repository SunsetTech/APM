#pragma once

typedef struct {
	float WaveVelocity;
	float TransferEfficiency;
} Wave_CellParameters;

float Wave_ComputeNextValue(
	int Dimensions,
	const Wave_CellParameters* GridParameters,
	const float* Cells,
	int* Position, const unsigned int* Bounds,
	float SpacetimeDelta
); 
