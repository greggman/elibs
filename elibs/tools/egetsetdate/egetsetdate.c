/*************************************************************************
 *                                                                       *
 *                       EGGETSETDATE.CPP                                *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION
		Get a date from one file and copy to another

   PROGRAMMERS


   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/15/96 : Created.

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna\ensure.h"

#include "echidna\argparse.h"
#include "echidna\eerrors.h"
#include "echidna\eio.h"
#include "echidna\strings.h"

/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/


/************************** P R O T O T Y P E S **************************/


/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/


/**************************** R O U T I N E S ****************************/

#define ARG_SOURCE		(newargs[ 0])
#define ARG_DEST		(newargs[ 1]) 

char Usage[] = "Usage: BGGETSETDATE SOURCE DEST\n";

ArgSpec Template[] = {
{STANDARD_ARG|REQUIRED_ARG,				"SOURCE",		"\tSOURCE = file to get date from\n", },
{STANDARD_ARG|REQUIRED_ARG,				"DEST",			"\tDEST   = file to set date\n", },
{0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
BEGINFUNCMAIN(main)
{
	char			**newargs;
	int				  success = FALSE;

	newargs = argparse (argc, argv, Template);

	if (!newargs)
	{
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		return EXIT_FAILURE;
	}
	else
	{
		FileDateType	fdt;
		
		if (EIO_GetFileDate (ARG_SOURCE, &fdt))
		{
			if (EIO_SetFileDate (ARG_DEST, &fdt))
			{
				success = TRUE;
			}
		}
	}

	if (GlobalErr) {
		EL_printf ("%s\n", GlobalErrMsg);
		return EXIT_FAILURE;
	}

	return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}
ENDFUNCMAIN(main)


