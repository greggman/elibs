/*
 * LARGEST.C
 *
 *  COPYRIGHT : 1990 Echidna.
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 05/09/90
 *   MODIFIED : 05/30/93
 *       TABS : 05 09
 *
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		Takes one file, gets the size and if the size is larger than
 *		the size saved in a specified file it saves the new larger size
 *		to the specified file.
 *
 * HISTORY
 *	
 *
*/

#include <echidna\platform.h>
#include "switches.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <echidna\utils.h>
#include <echidna\eerrors.h>
#include <echidna\argparse.h>
#include <echidna\eio.h>
#include <echidna\strings.h>


/**************************** C O N S T A N T S ***************************/

/******************************** T Y P E S *******************************/

/****************************** G L O B A L S *****************************/

/******************************* M A C R O S ******************************/

/******************************* T A B L E S ******************************/

/***************************** R O U T I N E S ****************************/

/******************************** TEMPLATE ********************************/

#define ARG_TESTFILE		(newargs[ 0])
#define ARG_SIZEFILE		(newargs[ 1])
#define ARG_STRING			(newargs[ 2])

char Usage[] = "Usage: LARGEST TESTFILE SIZEFILE [STRING]\n";

ArgSpec Template[] = {
{STANDARD_ARG|REQUIRED_ARG,	"TESTFILE",	"\tTESTFILE = File to size\n", },
{STANDARD_ARG|REQUIRED_ARG,	"SIZEFILE",	"\tSIZEFILE = File to write size to\n", },
{STANDARD_ARG,				"STRING",	"\tSTRING   = String to print to file and to scan\n"
										"\t    (ex. \"FONTSIZE = %ld\"\n"
										, },
{0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
{
	char			**newargs;
	char			*string = "%ld";

	newargs = argparse (argc, argv, Template);

	if (!newargs) {
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		return EXIT_FAILURE;
	} else {
		long	newsize = 0;
		long	oldsize = 0;

		if (ARG_STRING)
		{
			string = ARG_STRING;
		}

		{
			int		fh;

			fh = EIO_ReadOpen (ARG_TESTFILE);
			if (fh == (-1))
			{
				FailMess ("Couldn't open file '%s'\n", ARG_TESTFILE);
			}

			newsize = EIO_FileLength (fh);
			EIO_Close (fh);
		}

		{
			FILE	*fp;

			fp = fopen (ARG_SIZEFILE, "r");
			if (fp)
			{

				fscanf (fp, string, &oldsize);
			}
		}

		if (newsize > oldsize)
		{
			FILE	*fp;

			fp = fopen (ARG_SIZEFILE, "w");

			fprintf (fp, string, newsize);
			fprintf (fp, "\n");

			fclose (fp);
		}
/************************************  ************************************/
	}

	if (GlobalErr) {
		EL_printf ("%s\n", GlobalErrMsg);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

