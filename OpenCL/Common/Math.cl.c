#include "Math.cl.h"

unsigned int Wrap(int a, int n) {
	return ((a % n) + n) % n;
}

float Clamp(float Min, float Val, float Max) {
	return min(max(Min,Val),Max);
}

unsigned int MapIndex(unsigned int Dimensions, const int* Position, const unsigned int* Size) {
	unsigned int Result = 0;
	unsigned int Multiplier = 1;
	for (int Dimension = Dimensions-1; Dimension >= 0; Dimension--) {
		Result = mad24(Wrap(Position[Dimension], Size[Dimension]), Multiplier, Result);
		Multiplier = mul24(Multiplier, Size[Dimension]);
	}
	
	return Result;
}
