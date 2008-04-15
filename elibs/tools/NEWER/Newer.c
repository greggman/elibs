/*
 * NEWER.C
 *
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
 *		Copyright (c) 1994-2008, Echidna
 *
 *		All rights reserved.
 *
 *		Redistribution and use in source and binary forms, with or
 *		without modification, are permitted provided that the following
 *		conditions are met:
 *
 *		* Redistributions of source code must retain the above copyright
 *		  notice, this list of conditions and the following disclaimer. 
 *		* Redistributions in binary form must reproduce the above copyright
 *		  notice, this list of conditions and the following disclaimer
 *		  in the documentation and/or other materials provided with the
 *		  distribution. 
 *
 *		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *		CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 *		INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *		MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *		DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 *		BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *		EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *		TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *		DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *		ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *		OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *		OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *		POSSIBILITY OF SUCH DAMAGE.
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

