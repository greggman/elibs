/*
 * MUTATE.C
 *
 *  COPYRIGHT : 1993 Echidna.
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 09/11/93
 *   MODIFIED : 09/11/93
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
 *		Takes an bunch of files and renames them using numbers!
 *
 * HISTORY
 *	
 * TODO
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <echidna/utils.h>
#include <echidna/eerrors.h>
#include <echidna/argparse.h>
#include <echidna/strings.h>
#include <echidna/listapi.h>
#include <echidna/eio.h>

/**************************** C O N S T A N T S ***************************/

/******************************** T Y P E S *******************************/

/****************************** G L O B A L S *****************************/

int			 Test      = FALSE;

/******************************* M A C R O S ******************************/

/******************************* T A B L E S ******************************/

/***************************** R O U T I N E S ****************************/

/******************************** TEMPLATE ********************************/

#define ARG_FILESPEC	(newargs[ 0])
#define ARG_INFILTER	(newargs[ 1])
#define ARG_OUTFILTER	(newargs[ 2])
#define ARG_TEST		(newargs[ 3])

char Usage[] =	"Usage: MUTATE FILESPEC INFILTER OUTFILTER\n"
				"\n"
				"Example:\n"
				"\tMUTATE V.* V.%d V%d.TGA\n"
				"\n"
				"\t\tV.1 -> V1.TGA\n"
				"\t\tV.2 -> V2.TGA\n"
				"\t\tV.3 -> V3.TGA\n"
				"\n"
				"NOTE: It is CASE SENSITIVE!!!\n"
				"\n"
				;

ArgSpec Template[] = {
{STANDARD_ARG|REQUIRED_ARG,				"FILESPEC",		"\tFILESPEC  = File Spec (eg. 'V*.*')\n", },
{STANDARD_ARG|REQUIRED_ARG,				"INFILTER",		"\tINFILTER  = Input Filter (eg. 'V.%d')\n", },
{STANDARD_ARG|REQUIRED_ARG,				"OUTFILTER",	"\tOUTFILTER = Output Filter (eq. 'V%d.TGA')\n", },
{CHRSWITCH_ARG,							"T",			"\t-T        = Test (don't actually rename)\n", },
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
		LST_LIST	*fileList;

		Test = SWITCH_VALUE(ARG_TEST);

//		strupr (ARG_INFILTER);
//		strupr (ARG_OUTFILTER);

		fileList = EIO_GetFileList (ARG_FILESPEC, FALSE, FALSE);
		if (fileList)
		{
			LST_NODE	*nd;

			nd = LST_Head (fileList);
			while (!LST_EndOfList (nd))
			{
				char	newname[EIO_MAXPATH];
				char	oldname[EIO_MAXPATH];
				char	newspec[EIO_MAXPATH];
				int		value = 0;

			//	strupr (NodeName (nd));
				strcpy (oldname, EIO_Filename(LST_NodeName(nd)));

				sscanf (oldname, ARG_INFILTER, &value);
				sprintf (newname, ARG_OUTFILTER, value);

				EIO_fnmerge (newspec, EIO_Path(LST_NodeName(nd)), newname, NULL);

				if (Test)
				{
					EL_printf ("rename %s %s\n", LST_NodeName(nd), newspec);
				}
				else
				{
					rename (LST_NodeName(nd), newspec);
				}

				nd = LST_Next (nd);
			}
			LST_DeleteList (fileList);
		}
	}

	if (GlobalErr) {
		EL_printf ("%s\n", GlobalErrMsg);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

