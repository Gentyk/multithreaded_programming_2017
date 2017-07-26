//Ядро для четно-нечетной сортировки в OpenCl

__kernel void freaky_sort(__global int*buffer, __global int*result, __global int*size)
{
	
	int gid = get_global_id(0);
	int g_size = get_global_size(0);
	int length = *size;

	int i, j, n2, start,end;
	double l, g;
	l = length;
	g = g_size;
	double ss;
	n2 = length%g_size;
	ss = (l - n2) / g;//деление size/g_size с оруглением в минимальную сторону

	//если поток последний, то увеличим его массив на остаток
	if (gid == g_size-1)
	{
		n2 = (int)ss + n2;
	}
	else
		n2 = (int)ss;

	//вычисляем начало и конец для 
	start = (int)(length / g_size)*gid;
	end = start + n2;
	printf("\n! %d (%d):", gid, n2);

	//копируем исходный
	for (i = start; i<end; i += 1) {
		result[i] = buffer[i];
	}

	int sorted = 0;
	int x, y;
	while (sorted == 0)
	{
		sorted = 1;
		for (i = start; i<end - 1; i += 2)
		{
			if (result[i] > result[i + 1])
			{
				x = result[i];
				y = result[i + 1];
				result[i] = y;
				result[i + 1] = x;
			}
		}
		for (i = start + 1; i<end - 1; i += 2)
		{
			if (result[i] > result[i + 1])
			{
				x = result[i];
				y = result[i + 1];
				result[i] = y;
				result[i + 1] = x;
				sorted = 0;
			}
		}
	}
}
