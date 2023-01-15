////////////////////////////////////////////////////////////////////
//File: basic_environ.c
//
//Description: base file for environment exercises with openCL
//
// 
////////////////////////////////////////////////////////////////////
#define CL_TARGET_OPENCL_VERSION 210
#define cimg_use_jpeg
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#ifdef __APPLE__
  #include <OpenCL/opencl.h>
#else
  #include <CL/cl.h>
#endif
#include "CImg/CImg.h"


using namespace cimg_library;
  
// check error, in such a case, it exits

void cl_error(cl_int code, const char *string){
	if (code != CL_SUCCESS){
		printf("%d - %s\n", code, string);
	    exit(-1);
	}
}
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{

  clock_t time, total_time;
  time = clock();

  int err;                            	// error code returned from api calls
  size_t t_buf = 50;			// size of str_buffer
  char str_buffer[t_buf];		// auxiliary buffer	
  size_t e_buf;				// effective size of str_buffer in use

  const cl_uint num_platforms_ids = 10;				// max of allocatable platforms
  cl_platform_id platforms_ids[num_platforms_ids];		// array of platforms
  cl_uint n_platforms;						// effective number of platforms in use
  const cl_uint num_devices_ids = 10;				// max of allocatable devices
  cl_device_id devices_ids[num_platforms_ids][num_devices_ids];	// array of devices
  cl_uint n_devices[num_platforms_ids];				// effective number of devices in use for each platform
	
  cl_device_id device_id;             				// compute device id 
  cl_context context;                 				// compute context
  cl_command_queue command_queue;     				// compute command queue
    

  // 1. Scan the available platforms:
  err = clGetPlatformIDs (num_platforms_ids, platforms_ids, &n_platforms);
  cl_error(err, "Error: Failed to Scan for Platforms IDs");
  printf("Number of available platforms: %d\n\n", n_platforms);
	
  // 2. Scan for devices in each platform
  for (int i = 0; i < n_platforms; i++ ){
    err = clGetDeviceIDs( platforms_ids[i], CL_DEVICE_TYPE_ALL, num_devices_ids, devices_ids[i], &(n_devices[i]));
    cl_error(err, "Error: Failed to Scan for Devices IDs");
    printf("\t[%d]-Platform. Number of available devices: %d\n", i, n_devices[i]);
  }	
  // 3. Create a context, with a device
  cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platforms_ids[0], 0};
  context = clCreateContext(properties, n_devices[0], devices_ids[0], NULL, NULL, &err);
  cl_error(err, "Failed to create a compute context\n");
  device_id = devices_ids[0][0];

  // 4. Create a command queue
  cl_command_queue_properties proprt[] = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0 };
  command_queue = clCreateCommandQueueWithProperties(context, device_id, proprt, &err);
  cl_error(err, "Failed to create a command queue\n");

  // Calculate size of the file
  FILE *fileHandler = fopen("kernel_flip.cl", "r");
  fseek(fileHandler, 0, SEEK_END);
  size_t fileSize = ftell(fileHandler);
  rewind(fileHandler);

  // read kernel source into buffer
  char * sourceCode = (char*) malloc(fileSize + 1);
  sourceCode[fileSize] = '\0';
  fread(sourceCode, sizeof(char), fileSize, fileHandler);
  fclose(fileHandler);

  // create program from buffer
  cl_program program = clCreateProgramWithSource(context, 1, (const char**)&sourceCode, NULL, &err);
  cl_error(err, "Failed to create program with source\n");
  free(sourceCode);
  
  // Build the executable and check errors
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err != CL_SUCCESS){
    size_t len;
    char buffer[2048];

    printf("Error: Some error at building process.\n");
    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), &buffer, &len);
    printf("%s\n", buffer);
    
    exit(-1);
  }

	CImg<unsigned char> img_show("image.jpg");
  CImg<unsigned char> img("image.jpg");
	unsigned char* image_array = img.data();
  unsigned long size_img = sizeof(unsigned char)*img.height()*img.width()*img.depth()*3;
  printf("Img size: %d, %d, %d, %ld\n", img.height(), img.width(), img.depth(), size_img);
  
  // Create a compute kernel with the program we want to run
  cl_kernel kernel = clCreateKernel(program, "flip", &err);
  cl_error(err, "Failed to create kernel from the program\n");
  
  // Create OpenCL buffer visible to the OpenCl runtime
  cl_mem input  = clCreateBuffer(context, CL_MEM_READ_WRITE, size_img, NULL, &err);
  cl_error(err, "Failed to create memory buffer at device\n");
  

  cl_event kernel_event[3];
  // Write date into the memory object 
  err = clEnqueueWriteBuffer(command_queue, input, CL_TRUE, 0, size_img,
                             image_array, 0, NULL, &kernel_event[0]);
  cl_error(err, "Failed to enqueue a write command\n");
  
  int h_w[] = { img.height(), img.width(), img.width() * img.height() * img.spectrum() / 2};
  cl_error(err, "Failed to enqueue a write command\n");
  
  // Set the arguments to the kernel
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
  cl_error(err, "Failed to set argument 0\n");

  err = clSetKernelArg(kernel, 1, sizeof(int), &h_w[0]);
  cl_error(err, "Failed to set argument 1\n");

  err = clSetKernelArg(kernel, 2, sizeof(int), &h_w[1]);
  cl_error(err, "Failed to set argument 2\n");

  err = clSetKernelArg(kernel, 3, sizeof(int), &h_w[2]);
  cl_error(err, "Failed to set argument 3\n");

  // Launch Kernel
  size_t local_size = 64;
  size_t global_size = ((img.width() * img.height() * img.spectrum() / 2) / 64 + 1) * 64;
  printf("Sizes: %d, %d\n", global_size, local_size);
  err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, &kernel_event[1]);
  cl_error(err, "Failed to launch kernel to the device\n");
  
  err = clEnqueueReadBuffer(command_queue, input, CL_TRUE, 0, size_img,
                             image_array, 0, NULL, &kernel_event[2]);
  cl_error(err, "Failed to enqueue a write command\n");
  
  clWaitForEvents(3, kernel_event);

  cl_ulong time_start;
  cl_ulong time_end;
  clGetEventProfilingInfo(kernel_event[0], CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
  clGetEventProfilingInfo(kernel_event[0], CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
  double nanoSeconds = time_end-time_start; 
  printf("Write buffer execution time is: %0.3f milliseconds \n",nanoSeconds / 1000000.0);

  clGetEventProfilingInfo(kernel_event[1], CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
  clGetEventProfilingInfo(kernel_event[1], CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
  double nanoSeconds2 = time_end-time_start; 
  printf("OpenCl Kernel Execution time is: %0.3f milliseconds \n",nanoSeconds2 / 1000000.0);

  clGetEventProfilingInfo(kernel_event[2], CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
  clGetEventProfilingInfo(kernel_event[2], CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
  double nanoSeconds3 = time_end-time_start; 
  printf("Read buffer Execution time is: %0.3f milliseconds \n",nanoSeconds3 / 1000000.0);
  
  clReleaseMemObject(input);
  clReleaseProgram(program);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(command_queue);
  clReleaseContext(context);

  total_time = clock() - time;
  printf("Total time elapsed: %.4f ms\n", ((float)total_time)/CLOCKS_PER_SEC);

  FILE *fptr;
  // use appropriate location if you are using MacOS or Linux
  fptr = fopen("times.txt","a");

  if(fptr == NULL)
  {
    printf("Error!");   
    exit(1);             
  }

  fprintf(fptr,"%.4f %.4f %.4f %.4f\n",nanoSeconds / 1000000.0, nanoSeconds2 / 1000000.0, nanoSeconds3 / 1000000.0, ((float)total_time)/CLOCKS_PER_SEC);
  fclose(fptr);

  img_show.display();
  img.display();

  return 0;
}

