#include "Math.cl.hpp"

namespace Math {
	int Wrap(int a, int n) {
		return ((a % n) + n) % n;
	}

	float Clamp(float Min, float Val, float Max) {
		return min(max(Min,Val),Max);
	}
	
	namespace Map {
		unsigned int Index(unsigned int Dimensions, const int* Position, const int* Size) {
			unsigned int Result = 0;
			unsigned int Multiplier = 1;
			for (int Dimension = Dimensions-1; Dimension >= 0; Dimension--) {
				Result += Wrap(Position[Dimension], Size[Dimension]) * Multiplier;
				Multiplier *= Size[Dimension];
			}
			
			return Result;
		}
		
		unsigned int From2DTo1D(unsigned int X, unsigned int Y, __private unsigned int Dimensions[3]) {
			unsigned int Multiplier = Dimensions[1];
			
			return (
				+ Wrap(X,Dimensions[0]) * Multiplier
				+ Wrap(Y,Dimensions[1])
			);
		}

		unsigned int From3DTo1D(unsigned int X, unsigned int Y, unsigned int Z, __private unsigned int Dimensions[3]) {
			unsigned int Multiplier = Dimensions[1] * Dimensions[2];
			
			return (
				+ Wrap(X,Dimensions[0]) * Multiplier
				+ From2DTo1D(Y,Z,Dimensions+1)
			);
		}

		unsigned int From4DTo1D(unsigned int W, unsigned int X, unsigned int Y, unsigned int Z, __private unsigned int Dimensions[4]) {
			unsigned int Multiplier = Dimensions[1] * Dimensions[2] * Dimensions[3];
			
			return (
				+ Wrap(W,Dimensions[0]) * Multiplier
				+ From3DTo1D(X,Y,Z,Dimensions+1)
			);
		}

		unsigned int From5DTo1D(unsigned int V, unsigned int W, unsigned int X, unsigned int Y, unsigned int Z, __private unsigned int Dimensions[5]) {
			unsigned int Multiplier = Dimensions[1] * Dimensions[2] * Dimensions[3] * Dimensions[4];
			
			return (
				+ Wrap(V,Dimensions[0]) * Multiplier
				+ From4DTo1D(W,X,Y,Z,Dimensions+1)
			);
		}
	}
}

