/*************************************************************************
 *                                                                       *
 *                       EGCOMPARE.CPP                              *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION
		Copy files in alphabetical order

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

#define ARG_VERBOSE		(newargs[ 0])
#define ARG_FILE1		(newargs[ 1])
#define ARG_FILE2		(newargs[ 2]) 

char Usage[] = "Usage: BGCOMPARE FILE1 FILE2\n";

ArgSpec Template[] = {
{SWITCH_ARG,							"-V",			"\t-V    = Verbose\n", },
{STANDARD_ARG|REQUIRED_ARG,				"FILE1",		"\tFILE1 = file to compare\n", },
{STANDARD_ARG|REQUIRED_ARG,				"FILE2",		"\tFILE2 = file to compare\n", },
{0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
BEGINFUNCMAIN(main)
{
	char			**newargs;
	int				  fVerbose;
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
		int		infh1;
		int		infh2;

		fVerbose = SWITCH_VALUE(ARG_VERBOSE);
		
		infh1 = EIO_ReadOpen (ARG_FILE1);
		if (infh1 < 0)
		{
			goto notsame;
		}
		
		infh2 = EIO_ReadOpen (ARG_FILE2);
		if (infh2 < 0)
		{
			goto notsame;
		}
		
		if (EIO_FileLength(infh1) != EIO_FileLength(infh2))
		{
			goto notsame;
		}
		
		{
			static char buffer1[65536];
			static char buffer2[65536];
			
			int len1;
			int len2;
			
			do
			{
				len1 = EIO_Read (infh1, buffer1, sizeof (buffer1));
				len2 = EIO_Read (infh2, buffer2, sizeof (buffer2));
				
				if (len1 != len2) goto notsame;
				
				if (!len1)
				{
					break;
				}
				
				if (memcmp (buffer1, buffer2, len1))
				{
					goto notsame;
				}
			}
			while (len1 == sizeof (buffer1));			
			
			success = TRUE;
		}
	}
notsame:

	if (fVerbose)
	{
		EL_printf ("%s\n", success ? "Files are the same" : "Files are different");
	}

	if (GlobalErr) {
		EL_printf ("%s\n", GlobalErrMsg);
		return EXIT_FAILURE;
	}

	return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}
ENDFUNCMAIN(main)

