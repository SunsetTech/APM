#pragma once

typedef float Wave_ValueType;

typedef struct {
	Wave_ValueType WaveVelocity;
	Wave_ValueType TransferEfficiency;
} Wave_CellParameters;

Wave_ValueType Wave_Update(
	const int Dimensions,
	const Wave_CellParameters* GridParameters,
	Wave_ValueType* Spacetime,
	int* Position, 
	const unsigned int* Bounds,
	const Wave_ValueType SpacetimeDelta
); 
