#pragma once

#include <vector>
#include <CL/cl.h>
#include "Object/Base.hpp"

namespace APM {
	namespace Scene {
		struct Description {
			struct Connection {
				size_t SinkObjectID, SinkPlugID;
				size_t SourceObjectID, SourcePlugID;
			};
			
			std::vector<Object::Base*> Objects;
			std::vector<Connection> Connections;
		};
	}
}
