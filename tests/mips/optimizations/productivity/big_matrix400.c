
void main()
{
	int a[400][400], b[400][400], c[400][400];
	register int i, j, k, v;
	for (i = 0; i < 400; ++i)
	{
		for (j = 0; j < 400; ++j)
		{
			a[i][j] = i * j;
		}
	}

	for (i = 0; i < 400; ++i)
	{
		for (j = 0; j < 400; ++j)
		{
			b[i][j] = i + j;
		}
	}

	for (v = 0; v < 125; ++v)
	{
		for(i = 0; i < 400; ++i)
		{
		    for(j = 0; j < 400; ++j)
		    {
			register int cij = 0;
			for(k = 0; k < 400; ++k)
			    cij += a[i][k] * b[k][j];
			c[i][j] = cij;

		    }
		}
	}
	printf("%i\n", c[0][0]);	
}


