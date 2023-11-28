#pragma once

#include "Base.hpp"

#include <CL/cl.h>

#include "Spring.cl.h"

namespace APM::Scene::Object {
	class SpringBundle: public Base {
		public:
			cl_uint FiberCount, FiberLength, BufferLength;
			Spring_SpringParameters* SpringParameterBuffer;
			Spring_NodeParameters* NodeParameterBuffer;
			Spring_NodeState* SpaceBuffer;
			
			SpringBundle(cl_uint FiberCount, cl_uint FiberLength);
			size_t MapIndex(cl_uint Fiber, cl_uint Node);
			size_t GetTaskSize() override;
	};
}
