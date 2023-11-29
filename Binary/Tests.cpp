#define CL_TARGET_OPENCL_VERSION 200
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include <array>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cstring>

#include <CL/cl.h>
extern "C" {
	#include <unistd.h>
}

#include "CLUtils.hpp"
#include "Utils.hpp"
#include "Math.hpp"

#include "tinywav.h"

#include "FDM.cl.h"
#include "Spring.cl.h"

void Visualize2DSlice(cl_float* Buffer, const cl_uint Dimensions[2]) {
	cl_uint Cursor[2];
	for (cl_uint Y = 0; Y < Dimensions[1]; Y++) {
		Cursor[1] = Y;
		for (cl_uint X = 0; X < Dimensions[0]; X++) {
			Cursor[0] = X;
			cl_uint Index = Math::MapIndex<2>(Cursor, Dimensions);
			cl_float Value = Buffer[Index];
			cl_uint Brightness = Math::Clamp<cl_int>(0, floor((Value+1.0f)/2.0f*255.0f),255);
			printf("\x1b[38;2;%i;%i;%im█", Brightness, Brightness, Brightness);
		}
		printf("\x1b[0m\n");
	}
}

/*void VisualizeSampleBuffer(cl_float* Buffer, cl_uint GroupX, cl_uint GroupY, const cl_uint Dimensions[3]) {
	for (cl_uint SampleIndex = 0; SampleIndex < Dimensions[2]; SampleIndex++) {
		cl_float Cell = Buffer[Math::MapIndex::From3DTo1D(GroupX, GroupY, SampleIndex, Dimensions)];
		cl_uint Brightness = Math::Clamp<cl_int>(0,floor((Cell+1.0f)/2.0f*255.0f),255);
		printf("\x1b[38;2;%i;%i;%im█", Brightness, Brightness, Brightness);
	}
	printf("\x1b[0m\n");
}*/

void Test_2D_Buffered_SOG(cl_context Context, cl_device_id Device, cl_command_queue Queue) {
	/*srand((unsigned int)time(NULL));
	
	const cl_uint WorkPerGroupX = 16;
	const cl_uint WorkPerGroupY = 16;
	const cl_uint GroupWidth = 64/WorkPerGroupX;
	const cl_uint GroupHeight = 64/WorkPerGroupY;
	const cl_uint CellsPerWorkX = 1;
	const cl_uint CellsPerWorkY = 1;
	const cl_uint GridWidth = WorkPerGroupX*CellsPerWorkX;
	const cl_uint GridHeight = WorkPerGroupY*CellsPerWorkY;
	const cl_uint TotalWorkX = GroupWidth * WorkPerGroupX;
	const cl_uint TotalWorkY = GroupHeight * WorkPerGroupY;
	
	const cl_uint Iterations = 1024;
	const cl_uint IterationsBufferSize = 2;
	const cl_uint IterationsOnDevice = 128;
	
	const cl_uint ParameterBufferSize = GroupWidth * GroupHeight * GridWidth * GridHeight;
	const cl_uint ParameterDimensions[4] = {GroupWidth, GroupHeight, GridWidth, GridHeight};
	FDM_CellParameters* ParameterBuffer = (FDM_CellParameters*) calloc(ParameterBufferSize, sizeof(FDM_CellParameters));
	printf("ParameterBufferSize %lu kilobytes\n", ParameterBufferSize * sizeof(FDM_CellParameters) / 1024);
	
	const cl_uint WorkDimensions[5] = {GroupWidth, GroupHeight, IterationsBufferSize, GridWidth, GridHeight};
	const cl_uint WorkBufferSize = GroupWidth * GroupHeight * IterationsBufferSize * GridWidth * GridHeight;
	cl_float* WorkBuffer = (cl_float*) calloc(WorkBufferSize, sizeof(cl_float));
	printf("WorkBufferSize %lu kilobytes\n",WorkBufferSize * sizeof(cl_float) / 1024);
	
	const cl_uint SampleBufferSize = GroupWidth * GroupHeight * IterationsOnDevice;
	const cl_uint SampleBufferDimensions[3] = {GroupWidth, GroupHeight, IterationsOnDevice};
	cl_float* SampleBufferLeft = (cl_float*) calloc(SampleBufferSize, sizeof(cl_float));
	cl_float* SampleBufferRight = (cl_float*) calloc(SampleBufferSize, sizeof(cl_float));
	printf("SampleBufferSize %lu kilobytes\n", SampleBufferSize * sizeof(cl_float) / 1024);
	
	printf("Initializing Parameters and Initial State\n");
	
	for (cl_uint GroupX = 0; GroupX < GroupWidth; GroupX++) {
		for (cl_uint GroupY = 0; GroupY < GroupHeight; GroupY++) {
			for (cl_uint CellX = 0; CellX < GridWidth; CellX++) {
				for (cl_uint CellY = 0; CellY < GridHeight; CellY++) {
					ParameterBuffer[Math::MapIndex::From4DTo1D(GroupX,GroupY,CellX,CellY,ParameterDimensions)].WaveVelocity = powf(1.0f,2.0f);
					if (CellX == 0 || CellX == GridWidth-1 || CellY == 0 || CellY == GridHeight-1) {
						ParameterBuffer[Math::MapIndex::From4DTo1D(GroupX,GroupY,CellX,CellY,ParameterDimensions)].TransferEfficiency = 0.0f;
					} else {
						ParameterBuffer[Math::MapIndex::From4DTo1D(GroupX,GroupY,CellX,CellY,ParameterDimensions)].TransferEfficiency = 1.0f-(1.0f/4000.0f);
					}
				}
			}
			
			for (cl_uint ImpulseCounter = 0; ImpulseCounter < 5; ImpulseCounter++) {
				WorkBuffer[Math::MapIndex::From5DTo1D<cl_uint>(GroupX, GroupY, -1, rand(), rand(), WorkDimensions)] = 1;
			}
		}
	}

	cl_mem WorkBufferCL = clCreateBuffer(Context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * WorkBufferSize, WorkBuffer, &Err);CLUtils::PrintAndHaltIfError(Err);
	cl_mem ParameterBufferCL = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(FDM_CellParameters) * ParameterBufferSize, ParameterBuffer, &Err);CLUtils::PrintAndHaltIfError(Err);
	cl_mem SampleBufferLeftCL = clCreateBuffer(Context, CL_MEM_WRITE_ONLY, sizeof(cl_float) * SampleBufferSize, NULL, &Err);CLUtils::PrintAndHaltIfError(Err);
	cl_mem SampleBufferRightCL = clCreateBuffer(Context, CL_MEM_WRITE_ONLY, sizeof(cl_float) * SampleBufferSize, NULL, &Err);CLUtils::PrintAndHaltIfError(Err);
	
	printf("Setting Parameters\n");
	
	const CLUtils::ArgumentDefintion Arguments[] = {
		{sizeof(cl_mem ), &ParameterBufferCL },
		{sizeof(cl_mem ), &WorkBufferCL		  },
		{sizeof(cl_uint), &GroupWidth		  },
		{sizeof(cl_uint), &GroupHeight		  },
		{sizeof(cl_uint), &WorkPerGroupX	  },
		{sizeof(cl_uint), &WorkPerGroupY	  },
		{sizeof(cl_uint), &CellsPerWorkX	  },
		{sizeof(cl_uint), &CellsPerWorkY	  },
		{sizeof(cl_uint), &IterationsOnDevice },
		{sizeof(cl_mem ), &SampleBufferLeftCL },
		{sizeof(cl_mem ), &SampleBufferRightCL},
	};
	
	Err = CLUtils::SetKernelArguments(Kernel, std::size(Arguments), Arguments); CLUtils::PrintAndHaltIfError(Err);
	
	size_t globalSize[] = {TotalWorkX, TotalWorkY, 1};
	size_t localSize[] = {WorkPerGroupX, WorkPerGroupY, 1};
	
	cl_event ReadEvents[3];
	TinyWav WavOutputHandle;
	tinywav_open_write(
		&WavOutputHandle,
		2,
		44100,
		TW_FLOAT32,
		TW_SPLIT,
		"Output.wav"
	);
	
	printf("Executing\n");
	
	long long GlobalStartTime = Utils::Time::Microseconds();
	for (cl_uint Iteration = 0; Iteration < Iterations; Iteration++) {
		long long StartTime = Utils::Time::Microseconds();
		
		Err = clEnqueueNDRangeKernel(Queue, Kernel, 2, NULL, globalSize, localSize, 0, NULL, NULL);CLUtils::PrintAndHaltIfError(Err);
		
		Err = clEnqueueReadBuffer(Queue, SampleBufferLeftCL, CL_FALSE, 0, SampleBufferSize * sizeof(cl_float), SampleBufferLeft, 0, NULL, &ReadEvents[0]); CLUtils::PrintAndHaltIfError(Err);
		Err = clEnqueueReadBuffer(Queue, SampleBufferRightCL, CL_FALSE, 0, SampleBufferSize * sizeof(cl_float), SampleBufferRight, 0, NULL, &ReadEvents[1]); CLUtils::PrintAndHaltIfError(Err);
		Err = clEnqueueReadBuffer(Queue, WorkBufferCL, CL_FALSE, 0, WorkBufferSize * sizeof(cl_float), WorkBuffer, 0, NULL, &ReadEvents[2]); CLUtils::PrintAndHaltIfError(Err);
		
		clWaitForEvents(3, ReadEvents);
		
		clReleaseEvent(ReadEvents[0]);
		clReleaseEvent(ReadEvents[1]);
		clReleaseEvent(ReadEvents[2]);

		long long TotalTime = Utils::Time::Microseconds() - StartTime;
		cl_float* StereoBuffer[2] = {SampleBufferLeft, SampleBufferRight};

		tinywav_write_f(&WavOutputHandle, StereoBuffer, IterationsOnDevice);
		
		for (int Iteration = 0; Iteration < 2; Iteration++) {
			printf("\033[2J"); //Clear screen
			
			printf("Time taken: %lld us\n", TotalTime);
			printf("Per Sample: %lld us\n", TotalTime / IterationsOnDevice);
			printf("Time Spare: %lld us\n", IterationsOnDevice*22 - TotalTime);
			
			VisualizeSampleBuffer(SampleBufferLeft, 0, 0, SampleBufferDimensions);
			VisualizeSampleBuffer(SampleBufferRight, 0, 0, SampleBufferDimensions);
			VisualizeGrid(WorkBuffer, 0, 0, Iteration, WorkDimensions);
			
			usleep(33333);
		}
	}
	printf("Total time taken %lld\n", Utils::Time::Microseconds() - GlobalStartTime);
	printf("Cleaning Up\n");
	tinywav_close_write(&WavOutputHandle);
	*/
}

static void glfw_error_callback(int error, const char* description) {
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void Test_GUI() {
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) return;
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
	if (window == nullptr) return;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;	  // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	  // Enable Gamepad Controls

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");							// Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");				// Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);		// Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);			// Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))							// Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();
		}

		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);	// Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}
