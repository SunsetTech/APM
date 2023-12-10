#pragma once

#include "Typedefs.cl.h"

typedef cl_float Wave_PrecisionType;

Wave_PrecisionType Wave_Update(
	const cl_int Dimensions,
	const Wave_PrecisionType* WaveVelocity,
	const Wave_PrecisionType* TransferEfficiency,
	const Wave_PrecisionType* Spacetime,
	cl_int* Position, 
	const cl_uint* Bounds,
	const Wave_PrecisionType SpacetimeDelta
); 
