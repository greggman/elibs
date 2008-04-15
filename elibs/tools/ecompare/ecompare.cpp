/*************************************************************************
 *                                                                       *
 *                       EGCOMPARE.CPP                              	 *
 *                                                                       *
 *************************************************************************

		Copyright (c) 1996-2008, Echidna

		All rights reserved.

		Redistribution and use in source and binary forms, with or
		without modification, are permitted provided that the following
		conditions are met:

		* Redistributions of source code must retain the above copyright
		  notice, this list of conditions and the following disclaimer. 
		* Redistributions in binary form must reproduce the above copyright
		  notice, this list of conditions and the following disclaimer
		  in the documentation and/or other materials provided with the
		  distribution. 

		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
		CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
		INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
		MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
		DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
		BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
		EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
		TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
		DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
		ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
		OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
		OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
		POSSIBILITY OF SUCH DAMAGE.


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

