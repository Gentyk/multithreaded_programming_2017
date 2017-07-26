//Исспользование MPI
//Вычислить определенный интегралл,используя метод трапеций. Входные данные:
//Числа a и b, функцияf(x) определяется с помощью программной функции.
//При суммировании использовать принцип дихотомии.

#include <iostream>
#include <mpi.h>
#include <cstdlib>
#include <time.h>

#define N 100000000

float f(float x)
{
	float y;
	y = x*x*x+ 2*x - 3;
	return y;
}


int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);
	int rank, size, n, i, j, k, flag;
	unsigned long n1, n2;
	double t1, t2,a,b,h;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	//параметры формулы
	a = -10.0;
	b = 100.0;
	h = (b - a) / N;
	
	
	//для каждого потока выделяем память и формируем массив: он будет одинаковым
	if (size != 1)
		n1 = (N + size - N%size) / size;
	else
		n1 = N;
	double *data1 = (double *)malloc(n1 * sizeof(double));
	if (data1 == NULL)
	{ free(data1); printf("Memory (double)error rank=%d", rank); return 1; }
	for (i = 0; i < n1; i++)
		data1[i] = 0;

	MPI_Barrier(MPI_COMM_WORLD);
	if (rank == 0)
		t1 = MPI_Wtime();
	//заполнение
	i = rank*n1;
	while (i < (rank+1)*n1 && i<N)//не все будут заполнены
	{
		data1[i%n1] = h / 2 * (f(a + i*h) + f(a + (i + 1)*h));
		//printf("%f ", data1[i%n1]);
		i++;		
	}
	//printf("%f ", data1[i]);
	flag = 0;
	n2 = n1;
	while (n2 != 1)
	{
		for (i = 0; i < n2; i+=2)
		{			
			//printf("\n%d %f %f", rank, data1[i], data1[i + 1]);
			data1[int(i/2)] =data1[i]+data1[i + 1];
			data1[int(i + 1)] = 0;
		}
		n2 = (n2 + n2 % 2) / 2;
	}
	//printf("\nrank=%d, loc_summ=%f", rank, data1[0]);
	
	//if (rank != 0)
	//{
	//	printf("\nrank %d", rank);
	//	MPI_Send(&data1[0], 1, MPI_DOUBLE, 0, rank*10, MPI_COMM_WORLD);
	//}
	
	double ss[4];
	for (i = 0; i < 4; i++)
		ss[i] = 0;
	MPI_Barrier(MPI_COMM_WORLD);

	if (rank == 0)	
		t2 = MPI_Wtime();

	MPI_Gather(&data1[0], 1, MPI_DOUBLE, ss, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	

	MPI_Barrier(MPI_COMM_WORLD);
	
	if (rank == 0)
	{
		double sum=0;
		for (i = 0; i < 4; i++)
			sum += ss[i];
		t1 = t2 - t1;
		;
		printf("\nsumm=%.2f\ntime=%f",sum, t1);
	}
	//free(data1);
	//data1 = NULL;
	
	MPI_Finalize();
	return 0;
}
