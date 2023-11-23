#pragma once

#include "Math.cl.h"

typedef struct {
	float WaveVelocity;
	float TransferEfficiency;
} FDM_CellParameters;

float FDM_ComputeNextValue(
	int Dimensions,
	const __global FDM_CellParameters* GridParameters, unsigned int ParametersRegionStart,
	const __global float* Cells, unsigned int CellsRegionStart,
	int* Position, const unsigned int* Bounds,
	float SpacetimeDelta
); 
