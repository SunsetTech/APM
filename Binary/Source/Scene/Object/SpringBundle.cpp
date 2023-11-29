#include "SpringBundle.hpp"

#include "../../Math.hpp"

namespace APM::Scene::Object {
	SpringBundle::SpringBundle(cl_uint FiberCount, cl_uint FiberLength) {
		this->FiberCount = FiberCount;
		this->FiberLength = FiberLength;
		this->BufferLength = FiberCount * FiberLength;
		this->SpringParameterBuffer = new Spring_SpringParameters[this->BufferLength];
		this->NodeParameterBuffer = new Spring_NodeParameters[this->BufferLength];
		this->SpaceBuffer = new Spring_NodeState[this->BufferLength];
	}
	
	size_t SpringBundle::MapIndex(cl_uint Fiber, cl_uint Node) {
		cl_uint Cursor[] = {Fiber, Node};
		cl_uint Bounds[] = {this->FiberCount, this->FiberLength};
		return Math::MapIndex<2>(Cursor, Bounds);
	}
	
	size_t SpringBundle::GetTaskSize() {
		return this->BufferLength;
	}
	
	SpringBundle::~SpringBundle() {
		delete this->SpaceBuffer;
		delete this->NodeParameterBuffer;
		delete this->SpringParameterBuffer;
	}
}
