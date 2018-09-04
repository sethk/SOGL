#include <sys/types.h>
#include <stdio.h>
#include <math.h>

void
main(void)
{
	for (u_int width = 1; width < 10000; ++width)
	{
		for (u_int x = 0; x < width; x+= 1)
		{
			float fx = -1 + x * (2.0 / width);
			u_int outx = lround(((fx + 1.0) / 2.0) * (width));
			if (outx != x)
				fprintf(stderr, "(width = %u) %u => %g => %u\n",
						width, x, fx, outx);
			double dx = fx;
			u_int doutx = lround(((dx + 1.0) / 2.0) * (width));
			if (doutx != x)
				fprintf(stderr, "(width = %u) %u => %g => %u\n",
						width, x, dx, doutx);
		}
	}
}
