#include <CL/cl.h>

void Test_3D_Large(cl_context Context, cl_device_id Device, cl_command_queue Queue, unsigned int Iterations);
void Test_4D_Large(cl_context Context, cl_device_id Device, cl_command_queue Queue, unsigned int Iterations); 
void Test_Spring(cl_context Context, cl_device_id Device, cl_command_queue Queue, unsigned int Iterations);
void Test_GUI();
