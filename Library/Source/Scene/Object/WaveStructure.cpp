#include "WaveStructure.hpp"

#include "../../Math.hpp"

namespace APM::Scene::Object {
	WaveStructure::WaveStructure(cl_uint Dimensions, cl_uint* SpatialBounds) {
		this->Dimensions = Dimensions;
		this->SpatialBounds = SpatialBounds;
		this->BufferLength = 1;
		for (cl_uint Dimension = 0; Dimension < Dimensions; Dimension++) {
			this->BufferLength *= SpatialBounds[Dimension];
		}
		this->WaveVelocity = new Wave_PrecisionType[this->BufferLength];
		this->TransferEfficiency = new Wave_PrecisionType[this->BufferLength];
		this->SpaceBuffer = new Wave_PrecisionType[this->BufferLength];
	}
	
	size_t WaveStructure::MapIndex(cl_uint *Position) {
		return Math::MapIndex(this->Dimensions, Position, this->SpatialBounds);
	}
	
	size_t WaveStructure::GetTaskSize() {
		return this->BufferLength;
	}
	
	WaveStructure::~WaveStructure() {
		delete[] this->WaveVelocity;
		delete[] this->TransferEfficiency;
		delete[] this->SpaceBuffer;
	}
}
