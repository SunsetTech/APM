#pragma once

#include "Math.cl.hpp"

namespace FDM {
	typedef struct {
		float WaveVelocity;
		float TransferEfficiency;
	} CellParameters;
	
	inline float ComputeNextValue(
		int Dimensions,
		const __global CellParameters* GridParameters, unsigned int ParametersRegionStart,
		const __global float* Cells, unsigned int CellsRegionStart,
		int* Position, const int* Bounds,
		float SpacetimeDelta
	) {
		const int SpacetimeDimensions = Dimensions + 1;
		CellParameters Parameters = GridParameters[ParametersRegionStart + Math::Map::Index(Dimensions, Position+1, Bounds+1)];
		
		/*int Position[SpacetimeDimensions];
		for (unsigned int Dimension = 0; Dimension < SpacetimeDimensions; Dimension++) {
			Position[Dimension] = Position[Dimension];
		}*/
		
		float DoubleCurrent = 2.0f * Cells[CellsRegionStart + Math::Map::Index(SpacetimeDimensions,Position, Bounds)];
		Position[0]--;
		float Previous = Cells[CellsRegionStart + Math::Map::Index(SpacetimeDimensions,Position, Bounds)];
		Position[0]++;
		
		float Next = (float)Dimensions * -DoubleCurrent;
		for (unsigned int Dimension = 1; Dimension < SpacetimeDimensions; Dimension++) {
			Position[Dimension]--; 
			Next += Cells[CellsRegionStart + Math::Map::Index(SpacetimeDimensions,Position,Bounds)];
			Position[Dimension]+=2;
			Next += Cells[CellsRegionStart + Math::Map::Index(SpacetimeDimensions,Position,Bounds)];
			Position[Dimension]--;
		}
		
		Next *= SpacetimeDelta * Parameters.WaveVelocity; //TODO get WaveVelocity from parameters
		
		return Math::Clamp(-1.0f, (DoubleCurrent - Previous + Next) * Parameters.TransferEfficiency, 1.0f);
	}
}
