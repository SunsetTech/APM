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

    if (Dimensions == 1) {
        Result += Wrap(Position[0], Size[0]);
    } else if (Dimensions == 2) {
        Result += Wrap(Position[1], Size[1]) * Multiplier;
        Multiplier *= Size[1];
        Result += Wrap(Position[0], Size[0]) * Multiplier;
    } else if (Dimensions == 3) {
        Result += Wrap(Position[2], Size[2]) * Multiplier;
        Multiplier *= Size[2];
        Result += Wrap(Position[1], Size[1]) * Multiplier;
        Multiplier *= Size[1];
        Result += Wrap(Position[0], Size[0]) * Multiplier;
    } else {
        for (int Dimension = Dimensions - 1; Dimension >= 0; Dimension--) {
            Result += Wrap(Position[Dimension], Size[Dimension]) * Multiplier;
            Multiplier *= Size[Dimension];
        }
    }

    return Result;
}

