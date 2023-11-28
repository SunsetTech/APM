#include "SpringBundle.hpp"

#include "../../Math.hpp"

namespace APM::Scene::Object {
	SpringBundle::SpringBundle(cl_uint FiberCount, cl_uint FiberLength) {
		this->FiberCount = FiberCount;
		this->FiberLength = FiberLength;
		this->BufferLength = FiberCount * FiberLength;
		this->SpringParameterBuffer = (Spring_SpringParameters*)calloc(BufferLength, sizeof(Spring_SpringParameters));
		this->NodeParameterBuffer = (Spring_NodeParameters*)calloc(BufferLength, sizeof(Spring_NodeParameters));
		this->SpaceBuffer = (Spring_NodeState*)calloc(BufferLength, sizeof(Spring_NodeState));
	}
	
	size_t SpringBundle::MapIndex(cl_uint Fiber, cl_uint Node) {
		cl_uint Cursor[] = {Fiber, Node};
		cl_uint Bounds[] = {this->FiberCount, this->FiberLength};
		return Math::MapIndex<2>(Cursor, Bounds);
	}
	
	size_t SpringBundle::GetTaskSize() {
		return this->FiberCount * this->FiberLength;
	}
}
