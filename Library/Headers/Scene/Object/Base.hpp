#pragma once
#include <cstddef>
#include <CL/cl.h>

namespace APM::Scene::Object {
	class Base {
		public:
			virtual size_t GetTaskSize() =0;
	};
}
