__kernel void Test_IncrementLocation(
	__global unsigned int* Buffer
) {
	Buffer[get_global_id(0)]++;
}

__kernel void Test_DeviceEnqueue(
	const          unsigned int Iterations,
	const          unsigned int BufferSize,
	      __global unsigned int* Buffer
) {
	const ndrange_t WorkRange = ndrange_1D(BufferSize);
	clk_event_t StartEvent = create_user_event();
	set_user_event_status(StartEvent, CL_COMPLETE);
	clk_event_t ExecutionEvent;
	void (^IncrementBlock)(void) = ^{Test_IncrementLocation(Buffer);};
	for (unsigned int Iteration = 0; Iteration < Iterations; Iteration++) {
		enqueue_kernel(
			get_default_queue(),
			CLK_ENQUEUE_FLAGS_NO_WAIT,
			WorkRange,
			1, &StartEvent, &ExecutionEvent,
			IncrementBlock
		); release_event(StartEvent);
		StartEvent = ExecutionEvent;
	}
	release_event(StartEvent);
}
