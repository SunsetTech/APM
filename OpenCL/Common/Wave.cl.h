#pragma once

#include "Typedefs.cl.h"

typedef cl_float Wave_PrecisionType;

typedef struct {
	Wave_PrecisionType WaveVelocity;
	Wave_PrecisionType TransferEfficiency;
} Wave_CellParameters;

Wave_PrecisionType Wave_Update(
	const cl_int Dimensions, //TODO since these headers are compiled for both host and device we should probably provide portable typedefs for int/unsigned int/etc
	const Wave_CellParameters* GridParameters,
	Wave_PrecisionType* Spacetime,
	cl_int* Position, 
	const cl_uint* Bounds,
	const Wave_PrecisionType SpacetimeDelta
); 
