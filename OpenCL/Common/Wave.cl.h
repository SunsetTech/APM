#pragma once

typedef float Wave_ValueType;

typedef struct {
	Wave_ValueType WaveVelocity;
	Wave_ValueType TransferEfficiency;
} Wave_CellParameters;

void Wave_Update(
	int Dimensions,
	const Wave_CellParameters* GridParameters,
	Wave_ValueType* Spacetime,
	int* Position, const unsigned int* Bounds,
	Wave_ValueType SpacetimeDelta
); 
