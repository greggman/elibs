/*
 * SIZEFILE.C
 *
 *  COPYRIGHT : 1992 Echidna.
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 08/27/92
 *   MODIFIED : 08/27/92
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		Write the size of a file to another file.
 *
 * HISTORY
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include <echidna/eerrors.h>
#include <echidna/argparse.h>
#include <echidna/eio.h>

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/******************************** TEMPLATE ********************************/

#define ARG_INFILE			(newargs[ 0])
#define ARG_OUTFILE			(newargs[ 1])

char Usage[] = "Usage: Sizefile INFILE OUTFILE [switches...]\n";

ArgSpec Template[] = {
{	STANDARD_ARG | REQUIRED_ARG,	"INFILE",	"\tINFILE        = Data file\n", },
{	STANDARD_ARG | REQUIRED_ARG,	"OUTFILE",	"\tOUTFILE       = Size File\n", },
{	0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
{
	char			**newargs;
	long			 size;
	int				 fh;

	newargs = argparse (argc, argv, Template);

	if (!newargs) {
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		return EXIT_FAILURE;
	} else {

		fh = EIO_ReadOpen (ARG_INFILE);
		if (fh == (-1))
		{
			EL_printf ("Error, couldn't open infile '%s'\n", ARG_INFILE);
	 		return EXIT_FAILURE;
		}
		size = EIO_FileLength (fh);
		EIO_Close (fh);
			
		fh = EIO_WriteOpen (ARG_OUTFILE);
		if (fh == (-1))
		{
			EL_printf ("Error, couldn't open outfile '%s'\n", ARG_OUTFILE);
	 		return EXIT_FAILURE;
		}
				
		EL_printf ("Data file size = %ld\n", size);

		EIO_Write (fh, &size, sizeof (size));
		EIO_Close (fh);

	}

	return EXIT_SUCCESS;
}

