
void main()
{
	int a[100][100], b[100][100], c[100][100];
	register int i, j, k, v;
	for (i = 0; i < 100; ++i)
	{
		for (j = 0; j < 100; ++j)
		{
			a[i][j] = i * j;
		}
	}

	for (i = 0; i < 100; ++i)
	{
		for (j = 0; j < 100; ++j)
		{
			b[i][j] = i + j;
		}
	}

	for (v = 0; v < 8000; ++v)
	{
		for(i = 0; i < 100; ++i)
		{
		    for(j = 0; j < 100; ++j)
		    {
			register int cij = 0;
			for(k = 0; k < 100; ++k)
			    cij += a[i][k] * b[k][j];
			c[i][j] = cij;

		    }
		}
	}
	printf("%i\n", c[0][0]);	
}


