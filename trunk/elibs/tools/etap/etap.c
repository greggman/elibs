#include <stdio.h>
#include <stdlib.h>

int main (int argc, char** argv)
{
	if (argc != 2)
	{
		fprintf (stderr, "Error: Bad Args\n");
		fprintf (stderr, "Usage: etap filename\n");
		return EXIT_FAILURE;
	}
	else
	{
		FILE*	fp;

		fp = fopen (argv[1], "w");
		if (!fp)
		{
			fprintf (stderr, "Error: Couldn't open file '%s'\n", argv[1]);
			return EXIT_FAILURE;
		}

		{
			int	c;

			while ((c = getc (stdin)) != EOF)
			{
				putc (c, stdout);
				putc (c, fp);
				if (c == '\n')
				{
					fflush (fp);
					fflush (stdout);
				}
			}
		}
	}

	return EXIT_SUCCESS;
}
