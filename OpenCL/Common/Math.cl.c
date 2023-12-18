#include "Math.cl.h"

unsigned int Wrap(int a, int n) {
	return ((a % n) + n) % n;
	/*if (a < 0) {return n+a;}
	else if (a >= n) {return a-n;}
	else {return a;}*/
}

float Clamp(float Min, float Val, float Max) {
	return min(max(Min,Val),Max);
}

void ComputeMultipliers(unsigned int Dimensions, const unsigned int * restrict Bounds, unsigned int * restrict Multipliers) {
	unsigned int Multiplier = 1;
	for (int Dimension = Dimensions-1; Dimension >= 0; Dimension--) {
		Multipliers[Dimension] = Multiplier;
		Multiplier = mul24(Multiplier, Bounds[Dimension]);
	}
}

unsigned int MapIndexND(unsigned int Dimensions, const int* restrict Position, const unsigned int* restrict Bounds) {
	unsigned int Result = 0;
	unsigned int Multiplier = 1;
	for (int Dimension = Dimensions-1; Dimension >= 0; Dimension--) {
		Result = mad24(Wrap(Position[Dimension], Bounds[Dimension]), Multiplier, Result);
		Multiplier = mul24(Multiplier, Bounds[Dimension]);
	}
	
	return Result;
}

unsigned int MapIndex2D(
	cl_int X, cl_uint MultiplierX,
	cl_int Y
) {
	return X * MultiplierX + Y;
}

cl_uint MapIndex3D(
	const cl_int X, const cl_uint MultiplierX,
	const cl_int Y, const cl_uint MultiplierY,
	const cl_int Z
) {
	return X * MultiplierX + Y * MultiplierY + Z;
}
