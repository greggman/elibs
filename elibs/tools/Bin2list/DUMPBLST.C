#include <stdio.h>
#include <stdlib.h>
#include <echidna/etypes.h>
#include <echidna/readiff.h>
#include <echidna/eio.h>

int main (int argc, char **argv)
{
	int		 fh;
	ULONG	 offset;
	ULONG	 filelen;
	ULONG	 last = 0;
	ULONG	 len;
	short	 count = 0;
	short	 done = FALSE;

	argc = argc;

	fh = EIO_ReadOpen (argv[1]);
	if (fh == (-1))
	{
		EL_printf ("Error: Couldn't open %s\n", argv[1]);
		return (EXIT_FAILURE);
	}

	filelen = EIO_FileLength (fh);

	while (!done && EIO_Read (fh, &offset, sizeof (ULONG)) == sizeof (ULONG))
	{
		LongSex (offset);

		if (!offset)
		{
			offset = filelen;
			done = TRUE;
		}

		if (offset > last)
		{
			len = offset - last;

			if (count)
			{
				EL_printf ("#%-3d  size = %-6ld\n", count, len);
			}
			last = offset;
		}
		else
		{
			if (count)
			{
				EL_printf ("#%-3d  repeat", count);
			}
		}
		count++;
	}
}

