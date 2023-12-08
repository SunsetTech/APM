#pragma once

#include "Typedefs.cl.h"

typedef cl_float Spring_PrecisionType;

typedef struct {
	Spring_PrecisionType RestLength;
	Spring_PrecisionType Stiffness;
} Spring_SpringParameters;

typedef struct {
	Spring_PrecisionType Mass, Damping;
	cl_bool Fixed;
} Spring_NodeParameters;

typedef struct {
	Spring_PrecisionType Position;
	Spring_PrecisionType Velocity;
} Spring_NodeState;

Spring_NodeState Spring_NextState(
	const Spring_SpringParameters* SpringParameters, //0
	const Spring_NodeParameters* NodeParameters, 
	const Spring_NodeState* Spacetime,
	const cl_uint* SpacetimeBounds,
	cl_uint MassID,
	cl_uint Timestep, //4
	Spring_PrecisionType TimeDelta //5
);
