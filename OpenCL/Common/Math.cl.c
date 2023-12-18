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

unsigned int MapIndex(unsigned int Dimensions, const int* restrict Position, const unsigned int* restrict Size, local const unsigned int* restrict Multipliers) {
	unsigned int Result = 0;
	for (int Dimension = Dimensions-1; Dimension >= 0; Dimension--) {
		Result = mad24(Wrap(Position[Dimension], Size[Dimension]), Multipliers[Dimension], Result);
	}
	
	return Result;
}

unsigned int MapIndexSlow(unsigned int Dimensions, const int* restrict Position, const unsigned int* restrict Bounds) {
	unsigned int Result = 0;
	unsigned int Multiplier = 1;
	for (int Dimension = Dimensions-1; Dimension >= 0; Dimension--) {
		Result = mad24(Wrap(Position[Dimension], Bounds[Dimension]), Multiplier, Result);
		Multiplier = mul24(Multiplier, Bounds[Dimension]);
	}
	
	return Result;
}
