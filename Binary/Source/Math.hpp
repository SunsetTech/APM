#pragma once
#include <cmath>

namespace APM {
	namespace Math {
		template<typename NumberType> NumberType Wrap(NumberType a, NumberType n) {
			return ((a % n) + n) % n;
		}

		template<typename NumberType> NumberType Clamp(NumberType Min, NumberType Val, NumberType Max) {
			return fmin(fmax(Val,Min),Max);
		}
		
		template<unsigned int Dimensions, typename PositionType, typename BoundType> unsigned int MapIndex(const PositionType Position[Dimensions], const BoundType Bounds[Dimensions]) {//TODO make actually type safe somehow
			BoundType Result = 0;
			BoundType Multiplier = 1;
			for (int Dimension = Dimensions-1; Dimension >= 0; Dimension--) {
				Result += Wrap(Position[Dimension], (PositionType)Bounds[Dimension]) * Multiplier;
				Multiplier *= Bounds[Dimension];
			}
			
			return Result;
		}
		
		template<unsigned int Dimensions, typename BoundType> BoundType GridBufferSize(const BoundType Bounds[Dimensions]) {
			BoundType Size = 1;
			for (unsigned int Dimension = 0; Dimension < Dimensions; Dimension++) {
				Size *= Bounds[Dimension];
			}
			
			return Size;
		}
	}
}
