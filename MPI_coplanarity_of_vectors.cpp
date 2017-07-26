//использование MPI
//Задание:
//Найти все возможные тройки компланарных векторов. Входные
//данные: множество не равных между собой векторов(x, y, z),
//где x, y, z– числа. 
//Для рандомизации использовался вихрь Мерсена



#include <iostream>
#include <mpi.h>
#include <cstdlib>
#include <time.h>

#define N 250

class M
{
private:
	int index,seed;
	int MT[624];
public:
	M(int seed)
	{
		M::index = 0;
		int i;
		for (i = 0; i < 624; i++)
			M::MT[i] = 0;
		M::seed = 0;
		M::MT[0] = seed;
		for (i = 1; i < 624; i++)
			M::MT[i] = 0xffffffff & (0x6c078965 * (M::MT[i - 1] ^ (M::MT[i - 1] >> 30)) + i);
	}

	void generate_numbers()
	{
		int i,y;
		for (i = 0; i < 624; i++)
		{
			y = (M::MT[i] & 0x80000000) + (M::MT[(i + 1) % 624] & 0x7fffffff);
			M::MT[i] = M::MT[(i + 397) % 624] ^ (y >> 1);
			if ((y % 2) != 0)
			      M::MT[i] = M::MT[i] ^ 2567483615;
		}
	}

	int  extract_number()
	{
		int y;
		if (M::index == 0)
			M::generate_numbers();
		y = M::MT[M::index];
	    //закалка
		y = y ^ (y >> 11);
		y = y ^ ((y << 7) & 0x9d2c5680);
		y = y ^ ((y << 15) & 0xefc60000);
		y = y ^ (y >> 18);

		M::index = (M::index + 1) % 624;
		return y;
	}

};



//определение компланарности:1 - если а,в,с -компланарны. Иначе - 0.
int coplanarity(int a0, int a1, int a2, int b0, int b1, int b2,
	int c0, int c1, int c2)
{
	int d=0;
	d = a0 * b1 * c2 + c0 * b2 * a1 + b0 * c1 * a2 - c0 * b1 * a2 - a1 * b0 * c2 - a0 * c1 * b2;
	if (d == 0)
		return 1;
	else
		return 0;
}


int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);
	int rank, size, n, i, j, k,flag;
	unsigned long n1, n2;
	double t1, t2;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	M Mersene(int(time(0)));

	//для каждого потока выделяем память и формируем массив: он будет одинаковым
	int *data = (int *)malloc(N*3*sizeof(int));
	if (*data == NULL)
	{ 
		free(data);printf("\nMemory error in rank:%d", rank);return 1; 
	}
	for (int i = 0; i < N*3; i+=3)
	{
		flag = 0;
		while (flag != 1)
		{
			data[i] = Mersene.extract_number()%256;
			data[i + 1] = Mersene.extract_number()%256;
			data[i + 2] = Mersene.extract_number()%256;
			j = 0;
			flag = 1;
			//printf("\n(%d,%d,%d)",
				//data[i + 0], data[i + 1], data[i + 2]);
			//проверка,чтобы были неравны
			/*while (j < i)
			{
				if (data[i] == data[j] && data[i+1] == data[j+1] && data[i+2] == data[j+2])
				{
					flag = 0;
					break;
				}
				j+=3;
			}*/
			
		}	
	}
	MPI_Barrier(MPI_COMM_WORLD);

	if (rank == 0)
		t1 = MPI_Wtime();
	MPI_Barrier(MPI_COMM_WORLD);

	printf("\nrank=%d:",rank);
	//анализ
	int m = 0;
	n1 = 0;
	for (i = rank; i < N - 2; i+=size)
		for (j = i + 1; j < N - 1; j++)
			for (k = j + 1; k < N; k++)
			{
				n1++;
				
				if (coplanarity(data[i + 0], data[i + 1], data[i + 2],
					data[j + 0], data[j + 1], data[j + 2], 
					data[k], data[k+1], data[k+2]) == 1)
				{
					printf("\n((%d,%d,%d),(%d,%d,%d),(%d,%d,%d))",
						data[i + 0], data[i + 1], data[i + 2],
						data[j + 0], data[j + 1], data[j + 2],
						data[k + 0], data[k + 1], data[k + 2]);
					m += 9;
				}

			}
	m = int(m / 9);
	printf("\nв итоге:\nrank=%d ; (найденно)m=%d ; (просмотрено)n1=%d", rank, m, n1);
	MPI_Barrier(MPI_COMM_WORLD);
	if (rank == 0)
	{
		t2 = MPI_Wtime();
		t1 = t2 - t1;
		printf("\ntime=%f", t1);
	}
	
	
	free(data);
	data = NULL;
	MPI_Finalize();
	return 0;
}
