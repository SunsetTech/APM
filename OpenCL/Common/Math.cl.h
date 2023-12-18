#pragma once

#include "Typedefs.cl.h"

cl_uint Wrap(cl_int a, cl_int n); 
cl_float Clamp(cl_float Min, cl_float Val, cl_float Max); 

void ComputeMultipliers(unsigned int Dimensions, const unsigned int * Bounds, unsigned int * Multipliers);
cl_uint MapIndex(cl_uint Dimensions, const cl_int* Position, const cl_uint* Bounds, cl_local const cl_uint* Multipliers);
cl_uint MapIndexSlow(cl_uint Dimensions, const cl_int* Position, const cl_uint* Bounds);
