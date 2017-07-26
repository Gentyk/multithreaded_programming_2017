//Работа с OpenMp
//Задание по варианту:
//Вычислить прямое произведение множеств А1, А2, А3, А4. Входные данные: множества чисел А1, А2, А3, А4, 
//мощности множеств могут быть не равны между собой и мощность каждого множества больше или равна1. 
//(тестировалось на 1,2,4 потоках)

#include <omp.h>
#include <stdio.h>
#include <locale.h> 
#include <time.h>
#include<windows.h>
#include<iostream>
#include<string>
#include <malloc.h>

#define N 10
#define NUM_THREADS 4



int rand_initialization(int* ar1,int n1, int* ar2,int n2, int* ar3,int n3, int* ar4, int n4)
{
#pragma omp parallel 
	{
		int i, k, j, m;
#pragma omp sections nowait 
		{

#pragma omp section
			{
				for (i = 0; i < n1; i++){ ar1[i] = rand() % 100 + 1; }
			//	printf("S1 -%d\n", omp_get_thread_num());
			}
#pragma omp section
			{
				for (j = 0; j < n2; j++){ ar2[j] = rand() % 200 + 100; }
				///printf("S2 -%d\n", omp_get_thread_num());
			}
#pragma omp section
			{
				for (k = 0; k < n3; k++){ ar3[k] = rand() % 300 + 200; }
				///printf("S3 -%d\n", omp_get_thread_num());
			}
#pragma omp section
			{
				for (m = 0; m < n4; m++){ ar4[m] = rand() % 400 + 300; }
				///printf("S4 -%d\n", omp_get_thread_num());
			}
		}
	
	}
	return 0;
}

int direct_work(int* ar1, int n1, int* ar2, int n2, int* ar3, int n3, int* ar4, int n4, int** resul)
{
		int i, j, k, t, number;
#pragma omp parallel for private(j,k,t,number)// schedule(dynamic, 35)
			for (i = 0; i < n1; i++)
			for (j = 0; j < n2; j++)
			for (k = 0; k < n3; k++)
			for (t = 0; t < n4; t++)
			{
			number=t+n4*k+n4*n3*j+n2*n3*n4*i;
			resul[number][0] = ar1[i];
			resul[number][1] = ar2[j];
			resul[number][2] = ar3[k];
			resul[number][3] = ar4[t];
			}
	return 0;
					
}

int print_arrays(int* ar1, int n1, int* ar2, int n2, int* ar3, int n3, int* ar4, int n4)
{
	if (n1 < 20 && n2 < 20 && n3 < 20 && n4 < 20)
	{
		int i; printf("\n A1: ");
		for (i = 0; i < n1; i++){ printf("%d, ", ar1[i]); }
		printf("\n A2: ");
		for (i = 0; i < n2; i++){ printf("%d, ", ar2[i]); }
		printf("\n A3: ");
		for (i = 0; i < n3; i++){ printf("%d, ", ar3[i]); }
		printf("\n A4: ");
		for (i = 0; i < n4; i++){ printf("%d, ", ar4[i]); }
	}
	return 0;
}

int print_result(int** R, int n)
{
	if (n < 700)
	{
		int i;
		printf("\nResult:");
		for (i = 0; i < n; i++)
		{
			printf("\n(%d, %d, %d, %d);",R[i][0],R[i][1],R[i][2],R[i][3]);
		}
		
	}
	return 0;
}


int main()
{
	setlocale(LC_ALL, "Rus");   //подключение русского языка
	printf("Лабораторная работа 3(ТМП)\n");
	int start_time, end_time, flag, n1, n4, n2, n3, n;

	int inp=0;
	
	// конечное время

	//printf("Выберите способ инициализации множеств:1-вручную,0 -константно.");
	//scanf_s("%d", &inp);
	if (inp == 1)
	{
		scanf_s("%d", &n1);
		scanf_s("%d", &n2);
		scanf_s("%d", &n3);
		scanf_s("%d", &n4);
	}
	else
	{
		n4 = N;		n1 = N;		n2 = N;		n3 = N;
	}

	//выделение памяти
	int *A1 = NULL;
	int *A2 = NULL;
	int *A3 = NULL;
	int *A4 = NULL;
	A1 = (int*)malloc(n1*sizeof(int));
	if (A1 == NULL)	{ free(A1);	return 1; }
	A2 = (int*)malloc(n2*sizeof(int));
	if (A2 == NULL)	{ free(A2);	return 1; }
	A3 = (int*)malloc(n3*sizeof(int));
	if (A3 == NULL)	{ free(A3);	return 1; }
	A4 = (int*)malloc(n4*sizeof(int));
	if (A4 == NULL)	{ free(A4);	return 1; }
	int r[4];
	n = n1*n2*n3*n4;
	int **result = (int **)malloc(n*sizeof(int*));
	if (result == NULL)	{ free(result);	return 1; }
	for (int i = 0; i < n; i++)
	{
		result[i] = (int*)malloc(4 * sizeof(int));
		if (result[i] == NULL)	{ free(result[i]);	return 1; }
	}
	double** a;
	int* m;
	m=(int *)malloc(sizeof(double*)*N);

	if ( m= NULL){ free(m); return 1; }
	a = (double**)m;
	for (int i = 0; i < N; i++)
	{
		printf("!1");
		m[i] = (int)(malloc(sizeof(double)*N));
		if (m[i] == NULL)	{ free(&m[i]);	return 1; }
	}
	(&m[0])[0] = 1.2;
	printf("\n%d\n", (&m[0])[0]);

	//omp_set_num_threads(NUM_THREADS);
	printf("good start");
	//рандомное задание
	rand_initialization(A1, n1, A2, n2, A3, n3, A4, n4);
	
	//вывод на экран множеств, если длины каждого меньше 20
	//print_arrays(A1, n1, A2, n2, A3, n3, A4, n4);

	start_time = GetTickCount(); // начальное время
	//вычисляем прямое произведение множеств
	//unsigned int start_time = clock();
	direct_work(A1, n1, A2, n2, A3, n3, A4, n4, result);
	//unsigned int end_time = clock(); // конечное время
	//double search_time = (end_time - start_time)/1000; // искомое время
	end_time = GetTickCount();  // конечное время
	double search_time = (end_time - start_time) / 1000; // искомое время

	//вывод результата
	//print_result(result, n);
	unsigned int search_time1 = (end_time - start_time);

	printf("%d mseconds\n", search_time1);

	//затираем всё
	free(A1); A1 = NULL;
	free(A2); A2 = NULL;
	free(A3); A3 = NULL;
	free(A4); A4 = NULL;
	free(result); result = NULL;

	system("pause");

}
