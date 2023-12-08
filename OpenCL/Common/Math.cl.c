#include "Math.cl.h"

int Wrap(int a, int n) {
	return ((a % n) + n) % n;
}

float Clamp(float Min, float Val, float Max) {
	return min(max(Min,Val),Max);
}

unsigned int MapIndex(unsigned int Dimensions, const int* Position, const unsigned int* Size) {
	unsigned int Result = 0;
	unsigned int Multiplier = 1;
	for (int Dimension = Dimensions-1; Dimension >= 0; Dimension--) {
		Result += Wrap(Position[Dimension], Size[Dimension]) * Multiplier;
		Multiplier *= Size[Dimension];
	}
	
	return Result;
}
