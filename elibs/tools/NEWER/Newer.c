/*
 * NEWER.C
 *
 *  COPYRIGHT : 1994 Echidna.
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 04/01/94
 *   MODIFIED : 06/07/94
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oO///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		
 *
 * HISTORY
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include <ctype.h>
#include <echidna/eio.h>
#include <echidna/eerrors.h>
#include <echidna/argparse.h>

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/*********************************************************************
 *
 * FileNewer
 *
 * SYNOPSIS
 *		int  FileNewer (char *source, char *target)
 *
 * PURPOSE
 *		
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int FileNewer (char *source, char *target)
{
	FileDateType	s_fdt;
	FileDateType	t_fdt;

	if (!EIO_GetFileDate (source, &s_fdt))
	{
		FailMess ("Error getting date of '%s'\n%s\n", source, GlobalErrMsg);
	}
	if (!EIO_FileExists (target))
	{
		return TRUE;
	}
	if (!EIO_GetFileDate (target, &t_fdt))
	{
		FailMess ("Error getting date of '%s'\n%s\n", target, GlobalErrMsg);
	}
	return (EIO_CmpDates (&s_fdt, &t_fdt) > 0);
} // FileNewer


/******************************** TEMPLATE ********************************/

#define ARG_SRCFILE		(newargs[ 0])
#define ARG_TARGFILE	(newargs[ 1])

char Usage[] =	"Usage: NEWER SRCFILE TARGFILE\n";

ArgSpec Template[] = {
{STANDARD_ARG|REQUIRED_ARG,				"INFILE",	"\tINFILE  = Source File\n", },
{STANDARD_ARG|REQUIRED_ARG,				"OUTFILE",	"\tOUTFILE = Target File\n", },
{0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
{
	char			**newargs;

	newargs = argparse (argc, argv, Template);

	if (!newargs) {
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		return EXIT_FAILURE;
	} else {
		if (FileNewer (ARG_SRCFILE, ARG_TARGFILE))
		{
			EL_printf ("%s", ARG_SRCFILE);
		}
		return EXIT_SUCCESS;
	}
}

