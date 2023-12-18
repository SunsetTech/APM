#pragma once

#include "Typedefs.cl.h"

typedef cl_float Wave_PrecisionType;

Wave_PrecisionType Wave_UpdateND(
	const cl_int SpatialDimensions,
	const Wave_PrecisionType* WaveVelocity,
	const Wave_PrecisionType* TransferEfficiency,
	const Wave_PrecisionType* Spacetime,
	cl_int* Position, 
	const cl_uint* SpacetimeBounds,
	const Wave_PrecisionType SpacetimeDelta
);

Wave_PrecisionType Wave_Update2D(
	const Wave_PrecisionType* WaveVelocity,
	const Wave_PrecisionType* TransferEfficiency,
	const Wave_PrecisionType* Spacetime,
	const          int T,
	const unsigned int StrideT,
	const unsigned int MaxT,
	const          int X,
	const unsigned int StrideX,
	const unsigned int MaxX,
	const          int Y,
	const unsigned int MaxY,
	const Wave_PrecisionType SpacetimeDelta
);
