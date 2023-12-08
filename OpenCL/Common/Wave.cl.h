#pragma once

#include "Typedefs.cl.h"

typedef cl_float Wave_PrecisionType;

typedef struct {
	Wave_PrecisionType WaveVelocity;
	Wave_PrecisionType TransferEfficiency;
} Wave_CellParameters;

Wave_PrecisionType Wave_Update(
	const cl_int Dimensions,
	const Wave_CellParameters* GridParameters,
	Wave_PrecisionType* Spacetime,
	cl_int* Position, 
	const cl_uint* Bounds,
	const Wave_PrecisionType SpacetimeDelta
); 
