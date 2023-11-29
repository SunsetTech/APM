#pragma once

#include "Base.hpp"

#include <vector>
#include <CL/cl.h>

#include "Spring.cl.h"

namespace APM::Scene::Object {
	class SpringBundle: public Base {
		public:
			struct Plug {
				cl_uint FiberID, NodeID;
				float Center;
			};
			cl_uint FiberCount, FiberLength, BufferLength;
			Spring_SpringParameters* SpringParameterBuffer;
			Spring_NodeParameters* NodeParameterBuffer;
			Spring_NodeState* SpaceBuffer;
			std::vector<Plug> Inputs, Outputs;
			
			SpringBundle(cl_uint FiberCount, cl_uint FiberLength);
			size_t MapIndex(cl_uint Fiber, cl_uint Node);
			size_t GetTaskSize() override;
			~SpringBundle();
	};
}
