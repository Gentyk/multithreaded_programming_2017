//Четно-нечетная сортировка

#include <stdio.h>
#include <stdlib.h>
#include "time.h"

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define GLOBAL_WORK_ITEMS 1
#define LOCAL_WORK_ITEMS 1
#define SIZE 100
#define MAX_NUMBER 1000
#define MAX_SOURCE_SIZE 0x100000

int*init_source_buffer_random(int size) {
	int*buf = (int*)malloc(size * sizeof(int));
	if (!buf) return NULL;

	time_t t;
	srand((unsigned)time(&t));

	for (int i = 0; i < size; i++)
	{
		buf[i] = rand() % MAX_NUMBER;
	}

	return buf;
}
//malloc с проверкой
int*init_result_buffer(int size)
{
	int*buf = (int*)malloc(size * sizeof(int));
	if (!buf) return NULL;

	for (int i = 0; i < size; i++)
	{
		buf[i] = -1;
	}
	return buf;
}

void print_buffer(int*buffer, int size) {
	int i,j,k,ost,rank,n,n2;
	
	printf("\n");
	for (i=0;i<size;i++)
		printf("%d ", buffer[i]);
	
	printf("\n");
	return;
}
//сливане отсортированных массивов в один массив
void good_buffer(int*buffer,int*result1, int size) {
	if (GLOBAL_WORK_ITEMS > 1) {
		int start[GLOBAL_WORK_ITEMS], end[GLOBAL_WORK_ITEMS];
		int i, j, k, ost, rank, n, n2, min, min_ind;
		printf("\n");

		//считаем размер каждого кусочка
		ost = size%GLOBAL_WORK_ITEMS;
		n = (int)((size - ost) / GLOBAL_WORK_ITEMS);
		i = 0;

		//для каждого "потока" определяем начало и конец его куска в общем массиве(уже отсортированном)
		for (rank = 0; rank < GLOBAL_WORK_ITEMS; rank++) {
			n2 = n;
			if (rank == GLOBAL_WORK_ITEMS - 1) {
				n2 = n2 + ost;
				start[rank] = end[rank - 1];
				end[rank] = size;
			}
			else
			{
				start[rank] = rank*n2;
				end[rank] = (rank + 1)*n2;
			}

		}
		printf("\n");

		//формируем новый массив
		for (i = 0; i < size; i++)
		{
			j = 0;
			//нахождение минимального 
			min = MAX_NUMBER;
			min_ind = 0;
			for (j = 0; j < GLOBAL_WORK_ITEMS; j++) {
				if (start[j] < end[j])//в строке есть ли еще не попнутые значения
					if (buffer[start[j]] < min)//проверяем на минимальность
					{
						min_ind = j;//находим номер минимальной строки
						min = buffer[start[j]];//обновляем минимальное значение
					}
			}
	//		printf("%d ", buffer[start[min_ind]]);
			result1[i] = buffer[start[min_ind]];
			start[min_ind] += 1;//сдвигаем индекс в строке откуда попнули 

		}
	}
	else
	{
		for (int i=0;i<SIZE;i++)
			result1[i] = buffer[i];

	
	}
	return;
}

int main() {
	printf("started running\n");
	clock_t start, stop;

	//формируем массивы
	int size = SIZE;
	int*source_buffer = init_source_buffer_random(SIZE);
	int*result_buffer = init_result_buffer(SIZE);
	int*new_result_buffer = init_result_buffer(SIZE);

	printf("START:\n");
	print_buffer(source_buffer, SIZE);//вывод
	printf("\n");
	printf("\n");


	/*считываение программы kernel.cl*/
	char *source_str;
	size_t source_size;

	FILE *fp = fopen("kernel.cl", "r");
	if (!fp)
	{
		fprintf(stderr, "Failed to load kernel.\n");
		return 1;
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);
	printf("kernel loading done\n");

	cl_int ret_code;
	cl_int numb_platforms;
	cl_int numb_devices;
	cl_device_id device_id = NULL;

	/*get info about platforms available*/
	ret_code = clGetPlatformIDs(0, NULL, &numb_platforms);//количество свобдных
	cl_platform_id*platforms = NULL;
	platforms = (cl_platform_id*)malloc(numb_platforms * sizeof(cl_platform_id));
	ret_code = clGetPlatformIDs(numb_platforms, platforms, NULL);//выделение

	/*get info about devices*/
	ret_code = clGetDeviceIDs(platforms[1], CL_DEVICE_TYPE_ALL, 1, &device_id, &numb_devices); 
	char name[256];
	ret_code = clGetDeviceInfo(device_id, CL_DEVICE_NAME, 128, name, NULL);
	printf("Detected OpenCL device: %s\n", name);

	/*содаем context*/
	cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret_code);

	/*создание очередей потоков*/
	cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret_code);

	//задание буфера(со свойствами)
	cl_mem source_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, SIZE * sizeof(int), NULL, &ret_code);
	cl_mem result_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, SIZE * sizeof(int), NULL, &ret_code);
	cl_mem size_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int), NULL, &ret_code);

	//инициализация аргументов
	ret_code = clEnqueueWriteBuffer(command_queue, source_mem_obj, CL_TRUE, 0, SIZE * sizeof(int), source_buffer, 0, NULL, NULL);
	ret_code = clEnqueueWriteBuffer(command_queue, result_mem_obj, CL_TRUE, 0, SIZE * sizeof(int), result_buffer, 0, NULL, NULL);
	ret_code = clEnqueueWriteBuffer(command_queue, size_mem_obj, CL_TRUE, 0, sizeof(int), &size, 0, NULL, NULL);


	/*создание kernel*/
	cl_program program = clCreateProgramWithSource(context, 1, (const char**)&source_str, (const size_t*)&source_size, &ret_code);
	ret_code = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	cl_kernel kernel = clCreateKernel(program, "freaky_sort", &ret_code);

	/*передача аргументов*/
	ret_code = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&source_mem_obj);
	ret_code = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&result_mem_obj);
	ret_code = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&size_mem_obj);

	size_t global_item_size = GLOBAL_WORK_ITEMS;
	size_t local_item_size = LOCAL_WORK_ITEMS;

	start = clock();

	/*запуск kernel function*/
	ret_code = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);

	/*выдача результата*/
	ret_code = clEnqueueReadBuffer(command_queue, result_mem_obj, CL_TRUE, 0, SIZE * sizeof(int), result_buffer, 0, NULL, NULL);
	//слияние результатов
	
	
	good_buffer(result_buffer, new_result_buffer, SIZE);
	stop = clock();
	printf("\ntime in seconds = %f\n", ((double)stop - (double)start) / CLOCKS_PER_SEC);
	
	printf("END:\n");
	
	print_buffer(new_result_buffer, SIZE);
	/*очистка*/
	ret_code = clFlush(command_queue);
	ret_code = clFinish(command_queue);
	ret_code = clReleaseKernel(kernel);
	ret_code = clReleaseProgram(program);
	ret_code = clReleaseMemObject(source_mem_obj);
	ret_code = clReleaseMemObject(result_mem_obj);
	ret_code = clReleaseMemObject(size_mem_obj);
	ret_code = clReleaseCommandQueue(command_queue);
	ret_code = clReleaseContext(context);
	free(source_buffer);
	free(result_buffer);

	getchar();
	return 0;
}
