#pragma once

namespace Math {
	int Wrap(int a, int n); 
	float Clamp(float Min, float Val, float Max); 
	
	namespace Map {
		unsigned int Index(unsigned int Dimensions, const int* Position, const int* Size);
		template <unsigned int Dimensions> unsigned int Index(const int Position[Dimensions], const unsigned int Size[Dimensions]) {
			unsigned int Result = 0;
			unsigned int Multiplier = 1;
			for (int Dimension = Dimensions-1; Dimension >= 0; Dimension--) {
				Result += Wrap(Position[Dimension], Size[Dimension]) * Multiplier;
				Multiplier *= Size[Dimension];
			}
			
			return Result;
		}
		
		unsigned int From2DTo1D(unsigned int X, unsigned int Y, __private unsigned int Dimensions[3]); 
		unsigned int From3DTo1D(unsigned int X, unsigned int Y, unsigned int Z, __private unsigned int Dimensions[3]);
		unsigned int From4DTo1D(unsigned int W, unsigned int X, unsigned int Y, unsigned int Z, __private unsigned int Dimensions[4]);
		unsigned int From5DTo1D(unsigned int V, unsigned int W, unsigned int X, unsigned int Y, unsigned int Z, __private unsigned int Dimensions[5]);
	}
}
