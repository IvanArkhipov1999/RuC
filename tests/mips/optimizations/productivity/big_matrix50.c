
void main()
{
	int a[50][50], b[50][50], c[50][50];
	register int i, j, k, v;
	for (i = 0; i < 50; ++i)
	{
		for (j = 0; j < 50; ++j)
		{
			a[i][j] = i * j;
		}
	}

	for (i = 0; i < 50; ++i)
	{
		for (j = 0; j < 50; ++j)
		{
			b[i][j] = i + j;
		}
	}

	for (v = 0; v < 16000; ++v)
	{
		for(i = 0; i < 50; ++i)
		{
		    for(j = 0; j < 50; ++j)
		    {
			register int cij = 0;
			for(k = 0; k < 50; ++k)
			    cij += a[i][k] * b[k][j];
			c[i][j] = cij;

		    }
		}
	}
	printf("%i\n", c[0][0]);	
}


