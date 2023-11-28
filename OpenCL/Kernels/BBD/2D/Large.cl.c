#include "Math.cl.h"

__kernel void BBD_2D_Large(
	__global float* Input,
	__global float* Buffer,
	__global unsigned int BufferBounds[3],
	unsigned int Timestep
) {
	const unsigned int LineIndex = get_global_id(0);
	const unsigned int SampleIndex = get_global_id(1);
	const int PreviousLeftCursor[] = {LineIndex, Timestep-1, SampleIndex-1};
	const int CurrentCursor[] = {LineIndex, Timestep, SampleIndex};
	
	if (SampleIndex == 0) {
		Buffer[MapIndex(3,CurrentCursor,BufferBounds)] = Input[LineIndex];
	} else {
		Buffer[MapIndex(3,CurrentCursor,BufferBounds)] = Buffer[MapIndex(2,PreviousLeftCursor,BufferBounds)];
	}
}

