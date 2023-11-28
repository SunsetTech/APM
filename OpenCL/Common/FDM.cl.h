#pragma once

typedef struct {
	float WaveVelocity;
	float TransferEfficiency;
} FDM_CellParameters;

float FDM_ComputeNextValue(
	int Dimensions,
	const FDM_CellParameters* GridParameters,
	const float* Cells,
	int* Position, const unsigned int* Bounds,
	float SpacetimeDelta
); 
