#pragma once

#include "Typedefs.cl.h"

cl_int Wrap(cl_int a, cl_int n); 
cl_float Clamp(cl_float Min, cl_float Val, cl_float Max); 

cl_uint MapIndex(cl_uint Dimensions, const cl_int* Position, const cl_uint* Size);
