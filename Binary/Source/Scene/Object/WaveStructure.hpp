#pragma once

#include <vector>

#include "Base.hpp"
#include "Wave.cl.h"

namespace APM::Scene::Object {
	class WaveStructure: public Base {
		public:
			struct Plug {
				cl_uint* Position;
			};
			
			std::vector<Plug> Inputs, Outputs;
			
			cl_uint Dimensions;
			cl_uint* SpatialBounds;
			cl_uint BufferLength;
			Wave_CellParameters* CellParameterBuffer;
			Wave_ValueType* SpaceBuffer;
			
			WaveStructure(cl_uint Dimensions, cl_uint* SpatialBounds);
			size_t MapIndex(cl_uint* Position);
			size_t GetTaskSize() override;
			~WaveStructure();
	};
}
