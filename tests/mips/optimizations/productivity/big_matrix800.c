
void main()
{
	int a[800][800], b[800][800], c[800][800];
	register int i, j, k, v;
	for (i = 0; i < 800; ++i)
	{
		for (j = 0; j < 800; ++j)
		{
			a[i][j] = i * j;
		}
	}

	for (i = 0; i < 800; ++i)
	{
		for (j = 0; j < 800; ++j)
		{
			b[i][j] = i + j;
		}
	}

	for (v = 0; v < 16; ++v)
	{
		for(i = 0; i < 800; ++i)
		{
		    for(j = 0; j < 800; ++j)
		    {
			register int cij = 0;
			for(k = 0; k < 800; ++k)
			    cij += a[i][k] * b[k][j];
			c[i][j] = cij;

		    }
		}
	}
	printf("%i\n", c[0][0]);	
}


