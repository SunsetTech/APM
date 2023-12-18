#include "Math.cl.h"

unsigned int Wrap(int a, int n) {
	return ((a % n) + n) % n;
}

unsigned int WrapBottom(int a, int n) {
	return a < 0 ? n+a : a;
}

unsigned int WrapTop(int a, int n) {
	return a%n;
}

float Clamp(float Min, float Val, float Max) {
	return min(max(Min,Val),Max);
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
	cl_int X, cl_int MultiplierX,
	cl_int Y
) {
	return mad24(X, MultiplierX, Y);
}

cl_uint MapIndex3D(
	const cl_int X, const cl_int MultiplierX,
	const cl_int Y, const cl_int MultiplierY,
	const cl_int Z
) {
	return MapIndex2D(X, MultiplierX, MapIndex2D(Y, MultiplierY, Z));
}
