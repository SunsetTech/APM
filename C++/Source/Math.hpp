#pragma once
#include <cmath>

namespace Math {
	template<typename NumberType> NumberType Wrap(NumberType a, NumberType n) {
		return ((a % n) + n) % n;
	}

	template<typename NumberType> NumberType Clamp(NumberType Min, NumberType Val, NumberType Max) {
		return fmin(fmax(Val,Min),Max);
	}
	
	namespace MapIndex {
		template<typename NumberType> NumberType From2DTo1D(NumberType X, NumberType Y, const NumberType Dimensions[3]) {
			NumberType Multiplier = Dimensions[1];
			
			return (
				+ Wrap(X,Dimensions[0]) * Multiplier
				+ Wrap(Y,Dimensions[1])
			);
		}

		template<typename NumberType> NumberType From3DTo1D(NumberType X, NumberType Y, NumberType Z, const NumberType Dimensions[3]) {
			NumberType Multiplier = Dimensions[1] * Dimensions[2];
			
			return (
				+ Wrap(X,Dimensions[0]) * Multiplier
				+ From2DTo1D(Y,Z,Dimensions+1)
			);
		}

		template<typename NumberType> NumberType From4DTo1D(NumberType W, NumberType X, NumberType Y, NumberType Z, const NumberType Dimensions[4]) {
			NumberType Multiplier = Dimensions[1] * Dimensions[2] * Dimensions[3];
			
			return (
				+ Wrap(W,Dimensions[0]) * Multiplier
				+ From3DTo1D(X,Y,Z,Dimensions+1)
			);
		}
		
		template<typename NumberType> NumberType From5DTo1D(NumberType V, NumberType W, NumberType X, NumberType Y, NumberType Z, const NumberType Dimensions[5]) {
			NumberType Multiplier = Dimensions[1] * Dimensions[2] * Dimensions[3] * Dimensions[4];
			
			return (
				+ Wrap(V,Dimensions[0]) * Multiplier
				+ From4DTo1D(W,X,Y,Z,Dimensions+1)
			);
		}
	}
}
