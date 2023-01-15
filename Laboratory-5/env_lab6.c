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
	
  cl_device_id device_id_amd;             				// compute device id 
  cl_device_id device_id_nvidia;             				// compute device id 
  cl_context contextAMD;                 				// compute context
  cl_context contextNVIDIA;                 				// compute context
  cl_command_queue command_queueAMD;     				// compute command queue
    

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

  # pragma region Context
  // 3. Create a context, with a device
  cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platforms_ids[0], 0};
  contextAMD = clCreateContext(properties, n_devices[0], devices_ids[0], NULL, NULL, &err);
  cl_error(err, "Failed to create a compute context\n");
  device_id_amd = devices_ids[0][0];

  cl_context_properties properties2[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platforms_ids[1], 0};
  contextNVIDIA = clCreateContext(properties2, n_devices[1], devices_ids[1], NULL, NULL, &err);
  cl_error(err, "Failed to create a compute context\n");
  device_id_nvidia = devices_ids[1][0];

  printf("%d,    %d \n", device_id_amd, device_id_nvidia);
  #pragma endregion Context

  # pragma region Command_queue
  // 4. Create a command queue
  cl_command_queue_properties proprt[] = { CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0};
  command_queueAMD = clCreateCommandQueueWithProperties(contextAMD, device_id_amd, proprt, &err);
  cl_error(err, "Failed to create a command queue 1\n");

  cl_command_queue command_queueNVIDIA = clCreateCommandQueueWithProperties(contextNVIDIA, device_id_nvidia, proprt, &err);
  cl_error(err, "Failed to create a command queue 2\n");
  #pragma endregion Command_queue

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

  # pragma region Create_program
  // create program from buffer
  cl_program programAMD = clCreateProgramWithSource(contextAMD, 1, (const char**)&sourceCode, NULL, &err);
  cl_error(err, "Failed to create program with source\n");

  cl_program programNVIDIA = clCreateProgramWithSource(contextNVIDIA, 1, (const char**)&sourceCode, NULL, &err);
  cl_error(err, "Failed to create program with source\n");
  free(sourceCode);

  # pragma endregion Create_program
  
  # pragma region Build_program
  // Build the executable and check errors
  err = clBuildProgram(programAMD, 0, NULL, NULL, NULL, NULL);
  if (err != CL_SUCCESS){
    size_t len;
    char buffer[2048];

    printf("Error: Some error at building process.\n");
    clGetProgramBuildInfo(programAMD, device_id_amd, CL_PROGRAM_BUILD_LOG, sizeof(buffer), &buffer, &len);
    printf("%s\n", buffer);
    
    exit(-1);
  }

  err = clBuildProgram(programNVIDIA, 0, NULL, NULL, NULL, NULL);
  if (err != CL_SUCCESS){
    size_t len;
    char buffer[2048];

    printf("Error: Some error at building process.\n");
    clGetProgramBuildInfo(programNVIDIA, device_id_nvidia, CL_PROGRAM_BUILD_LOG, sizeof(buffer), &buffer, &len);
    printf("%s\n", buffer);
    
    exit(-1);
  }
  # pragma endregion Build_program
  
  # pragma region Kernel/input
  // Create a compute kernel with the program we want to run
  cl_kernel kernelAMD = clCreateKernel(programAMD, "flip", &err);
  cl_error(err, "Failed to create kernel from the program\n");

    // Create a compute kernel with the program we want to run
  cl_kernel kernelNVIDIA = clCreateKernel(programNVIDIA, "flip", &err);
  cl_error(err, "Failed to create kernel from the program\n");
  # pragma endregion Kernel/input

  # pragma region Image
  CImg<unsigned char> img("wallpaper.jpg");
	unsigned char* image_array = img.data();
  unsigned long size_img = sizeof(unsigned char)*img.height()*img.width()*img.depth()*3;
  printf("Img size: %d, %d, %d, %ld\n", img.height(), img.width(), img.depth(), size_img);

  const int n_images = 20;
  //Replicar imagen
  // Crear vector con todas las imagenes
  unsigned char* images[n_images * 2]; 
  for(int i = 0; i < n_images * 2; i++){
    images[i] = (u_char*)malloc(size_img);
    memcpy(images[i], image_array, size_img);
  }

  # pragma endregion Image

  cl_event kernel_eventAMD[3][n_images];
  cl_event kernel_eventNVIDIA[3][n_images];
  # pragma region Prepare_buffers
  cl_mem inputAMD[n_images];
  cl_mem inputNVIDIA[n_images];
  int h_w[] = { img.height(), img.width(), img.width() * img.height() * img.spectrum() / 2};

  clock_t time;
  time = clock();

  for(int i = 0; i < n_images; i++){
    // Create OpenCL buffer visible to the OpenCl runtime
    inputAMD[i]  = clCreateBuffer(contextAMD, CL_MEM_READ_WRITE, size_img, NULL, &err);
    cl_error(err, "Failed to create memory buffer at device\n");
    
    // Create OpenCL buffer visible to the OpenCl runtime
    inputNVIDIA[i]  = clCreateBuffer(contextNVIDIA, CL_MEM_READ_WRITE, size_img, NULL, &err);
    cl_error(err, "Failed to create memory buffer at device\n");
    
    // Write date into the memory object 
    err = clEnqueueWriteBuffer(command_queueAMD, inputAMD[i], CL_FALSE, 0, size_img,
                              images[i], 0, NULL, &kernel_eventAMD[0][i]);
    cl_error(err, "Failed to enqueue a write command\n");

    // Write date into the memory object 
    err = clEnqueueWriteBuffer(command_queueNVIDIA, inputNVIDIA[i], CL_FALSE, 0, size_img,
                              images[i + n_images], 0, NULL, &kernel_eventNVIDIA[0][i]);
    cl_error(err, "Failed to enqueue a write command\n");
  }
  # pragma endregion Prepare_buffers

  size_t len;
  size_t buffer[2048];
  
  clGetKernelWorkGroupInfo( kernelAMD , device_id_amd, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(buffer), &buffer, &len);
  size_t preferred_size_amd = *buffer;
  size_t local_sizeAMD = preferred_size_amd;
  size_t global_sizeAMD = ((img.width() * img.height() * img.spectrum() / 2) / preferred_size_amd + 1) * preferred_size_amd;
  printf("Sizes: %d, %d\n", global_sizeAMD, local_sizeAMD);

  clGetKernelWorkGroupInfo( kernelNVIDIA , device_id_nvidia, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(buffer), &buffer, &len);
  size_t preferred_size_nvidia = *buffer;
  size_t local_sizeNVIDIA = preferred_size_nvidia;
  size_t global_sizeNVIDIA = ((img.width() * img.height() * img.spectrum() / 2) / preferred_size_nvidia + 1) * preferred_size_nvidia;
  printf("Sizes: %d, %d\n", global_sizeNVIDIA, local_sizeNVIDIA);

  clFinish(command_queueAMD);
  clFinish(command_queueNVIDIA);
  
  for(int i = 0; i < n_images; i ++){
    
    # pragma region Arguments
    
    // Set the arguments to the kernel
    err = clSetKernelArg(kernelAMD, 0, sizeof(cl_mem), &inputAMD[i]);
    cl_error(err, "Failed to set argument 0\n");

    err = clSetKernelArg(kernelAMD, 1, sizeof(int), &h_w[0]);
    cl_error(err, "Failed to set argument 1\n");

    err = clSetKernelArg(kernelAMD, 2, sizeof(int), &h_w[1]);
    cl_error(err, "Failed to set argument 2\n");

    err = clSetKernelArg(kernelAMD, 3, sizeof(int), &h_w[2]);
    cl_error(err, "Failed to set argument 3\n");

    
    // Set the arguments to the kernel
    err = clSetKernelArg(kernelNVIDIA, 0, sizeof(cl_mem), &inputNVIDIA[i]);
    cl_error(err, "Failed to set argument 0\n");

    err = clSetKernelArg(kernelNVIDIA, 1, sizeof(int), &h_w[0]);
    cl_error(err, "Failed to set argument 1\n");

    err = clSetKernelArg(kernelNVIDIA, 2, sizeof(int), &h_w[1]);
    cl_error(err, "Failed to set argument 2\n");

    err = clSetKernelArg(kernelNVIDIA, 3, sizeof(int), &h_w[2]);
    cl_error(err, "Failed to set argument 3\n");
    # pragma endregion Arguments

    // Launch Kernel
    err = clEnqueueNDRangeKernel(command_queueAMD, kernelAMD, 1, NULL, &global_sizeAMD, &local_sizeAMD, 0, NULL, &kernel_eventAMD[1][i]);
    cl_error(err, "Failed to launch kernel to the device\n");

    
    err = clEnqueueNDRangeKernel(command_queueNVIDIA, kernelNVIDIA, 1, NULL, &global_sizeNVIDIA, &local_sizeNVIDIA, 0, NULL, &kernel_eventNVIDIA[1][i]);
    cl_error(err, "Failed to launch kernel to the device\n");
  }
  
  clFinish(command_queueAMD);
  clFinish(command_queueNVIDIA);

  for(int i = 0; i < n_images ; i++){

    err = clEnqueueReadBuffer(command_queueAMD, inputAMD[i], CL_FALSE, 0, size_img,
                                images[i], 0, NULL, &kernel_eventAMD[2][i]);
    cl_error(err, "Failed to enqueue a write command\n");
    
    err = clEnqueueReadBuffer(command_queueNVIDIA, inputNVIDIA[i], CL_FALSE, 0, size_img,
                                images[i+n_images], 0, NULL, &kernel_eventNVIDIA[2][i]);
    cl_error(err, "Failed to enqueue a write command\n");
  }

  clFinish(command_queueAMD);
  clFinish(command_queueNVIDIA);

  time = clock() - time;
  
  clReleaseProgram(programAMD);
  clReleaseProgram(programNVIDIA);
  clReleaseKernel(kernelAMD);
  clReleaseKernel(kernelNVIDIA);
  clReleaseCommandQueue(command_queueAMD);
  clReleaseCommandQueue(command_queueNVIDIA);
  clReleaseContext(contextAMD);
  clReleaseContext(contextNVIDIA);

  double wr_buffer[2][n_images];
  double kernel_time[2][n_images];
  double rd_buffer[2][n_images];
  for(int i = 0; i < n_images; i++){

    clReleaseMemObject(inputAMD[i]);
    clReleaseMemObject(inputNVIDIA[i]);

    # pragma region Time_measurement

    cl_ulong time_start;
    cl_ulong time_end;

    // Amd ------------------------------

    clGetEventProfilingInfo(kernel_eventAMD[0][i], CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
    clGetEventProfilingInfo(kernel_eventAMD[0][i], CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
    wr_buffer[0][i] = time_end-time_start; 
    printf(" AMD Write buffer execution time is: %0.3f milliseconds \n",wr_buffer[0][i] / 1000000.0);

    clGetEventProfilingInfo(kernel_eventAMD[1][i], CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
    clGetEventProfilingInfo(kernel_eventAMD[1][i], CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
    kernel_time[0][i] = time_end-time_start; 
    printf(" AMD OpenCl Kernel Execution time is: %0.3f milliseconds \n",kernel_time[0][i]  / 1000000.0);

    clGetEventProfilingInfo(kernel_eventAMD[2][i], CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
    clGetEventProfilingInfo(kernel_eventAMD[2][i], CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
    rd_buffer[0][i] = time_end-time_start; 
    printf(" AMD Read buffer Execution time is: %0.3f milliseconds \n",rd_buffer[0][i] / 1000000.0);

    // Nvidia ------------------------------

    clGetEventProfilingInfo(kernel_eventNVIDIA[0][i], CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
    clGetEventProfilingInfo(kernel_eventNVIDIA[0][i], CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
    wr_buffer[1][i] = time_end-time_start; 
    printf(" NVIDIA Write buffer execution time is: %0.3f milliseconds \n",wr_buffer[1][i] / 1000000.0);

    clGetEventProfilingInfo(kernel_eventNVIDIA[1][i], CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
    clGetEventProfilingInfo(kernel_eventNVIDIA[1][i], CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
    kernel_time[1][i] = time_end-time_start; 
    printf(" NVIDIA OpenCl Kernel Execution time is: %0.3f milliseconds \n",kernel_time[1][i] / 1000000.0);

    clGetEventProfilingInfo(kernel_eventNVIDIA[2][i], CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
    clGetEventProfilingInfo(kernel_eventNVIDIA[2][i], CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
    rd_buffer[1][i] = time_end-time_start; 
    printf(" NVIDIA Read buffer Execution time is: %0.3f milliseconds \n",rd_buffer[1][i] / 1000000.0);
    # pragma endregion Time_measurement
  }

  FILE *fptr;
  // use appropriate location if you are using MacOS or Linux
  fptr = fopen("times_lab6.txt","a");

  if(fptr == NULL)
  {
    printf("Error!");   
    exit(1);             
  }

  printf("%.4f\n", ((float)time)/CLOCKS_PER_SEC);
  fprintf(fptr, "%.4f\n", ((float)time)/CLOCKS_PER_SEC);

  for(int p = 0; p < 2; p++){
    for(int i = 0; i < n_images; i++){
      if(p == 0)
        fprintf(fptr,"AMD %.4f %.4f %.4f\n",wr_buffer[p][i] / 1000000.0, kernel_time[p][i] / 1000000.0, rd_buffer[p][i] / 1000000.0);
      else
        fprintf(fptr,"NVIDIA %.4f %.4f %.4f\n",wr_buffer[p][i] / 1000000.0, kernel_time[p][i] / 1000000.0, rd_buffer[p][i] / 1000000.0);
    }
  }
  
  fclose(fptr);
  
  //img.display();
  for(int i = 0; i < n_images * 2; i++){
    char buffer[50];
    if(i == n_images * 2 - 1){
      printf("Nvidia Images\n");

      CImg<unsigned char> image(images[i], img.width(), img.height(), img.depth(), img.spectrum());
      //image.display();
      snprintf(buffer, 50, "images/nvidia/%s/nvidia_%s_%s.bmp", argv[1], argv[1], argv[2]);
      image.save(buffer);
    }else if(i == n_images - 1){
      printf("AMD Images\n");

      CImg<unsigned char> image(images[i], img.width(), img.height(), img.depth(), img.spectrum());
      //image.display();
      snprintf(buffer, 50, "images/amd/%s/amd_%s_%s.bmp", argv[1], argv[1], argv[2]);
      image.save(buffer);
    }
    
    //CImg<unsigned char> image(images[i], img.height(), img.width(), img.depth(), img.spectrum());
    //image.display();

    free(images[i]);
  }

  return 0;
}

