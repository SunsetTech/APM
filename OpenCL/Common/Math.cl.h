#pragma once

#include "Typedefs.cl.h"

cl_uint Wrap(cl_int a, cl_int n); 
cl_uint WrapBottom(cl_int a, cl_int n);
cl_uint WrapTop(cl_int a, cl_int n);
cl_float Clamp(cl_float Min, cl_float Val, cl_float Max); 

void ComputeMultipliers(unsigned int Dimensions, const unsigned int * Bounds, unsigned int * Multipliers);
cl_uint MapIndexND(cl_uint Dimensions, const cl_int* Position, const cl_uint* Size);
cl_uint MapIndex2D(
	cl_int X, cl_int MultiplierX, 
	cl_int Y
);
cl_uint MapIndex3D(
	cl_int X, cl_int MultiplierX,
	cl_int Y, cl_int MultiplierY,
	cl_int Z
);
