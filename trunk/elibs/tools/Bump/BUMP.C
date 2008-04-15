/*
 * BUMP.C
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.004
 *    CREATED : 01/29/92
 *   MODIFIED : 12/13/93
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 *		Copyright (c) 1986-2008, Echidna
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
 *
 * SEE ALSO
 *
 *		
 * TODO	
 *
*/

#include "platform.h"
#include "switches.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>
#include <echidna/argparse.h>
#include <echidna/checkglu.h>
#include <echidna/eerrors.h>

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

char	*template = "char Version[] = \"00.";

/******************************* M A C R O S ******************************/


/******************************* T A B L E S ******************************/


/***************************** R O U T I N E S ****************************/


/******************************** TEMPLATE ********************************/

#define	ARG_FILENAME	(newargs[ 0])
#define	ARG_TEMPLATE	(newargs[ 1])
#define ARG_HELP		(newargs[ 2])

#define PROG	"BUMP"

char Usage[] = "Usage: " PROG " FILENAME [TEMPLATE]\n";

ArgSpec Template[] = {
	{ STANDARD_ARG|REQUIRED_ARG, "FILENAME", "\tFILENAME = .C or .H version file\n", },
	{ STANDARD_ARG,				 "TEMPLATE", "\tTEMPLATE = Scaning Template\n", },
	{ CHRSWITCH_ARG,	"?",	"", },
	{ 0, NULL, NULL, },
};

/*********************************************************************
 *
 * main
 *
 * PURPOSE
 *		Parse command line and make call to DoConversion.
 *
*/
int main (
	int		argc,
	char	**argv
)
{
	short	result = TRUE;
	char	**newargs;

	EL_printf (PROG " Copyright (c) 1992, 1993 Echidna\n");
	newargs = argparse (argc, argv, Template);
	if (!newargs || SWITCH_VALUE(ARG_HELP))
	{
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		return (EXIT_FAILURE);
	}

	if (ARG_TEMPLATE)	template = ARG_TEMPLATE;

	{
		FILE	*fp;
		char	 line[80];
		int		 value = 0;

		fp = fopen (ARG_FILENAME, "r");
		if (fp)
		{
			if (fgets (line, 80, fp))
			{
				char	*s;
				size_t	 len;

				len = strlen (line);
				if (len)
				{
					s = line + len - 1;
					while (len && !isdigit (*s))
					{
						s--;
						len--;
					}

					while (len && isdigit (*s))
					{
						s--;
						len--;
					}
					s++;

					value = atoi (s) + 1;
					template = line;
					line[len] = '\0';
				}
			}
			fclose (fp);
		}

		{
			time_t	tm;
			char	*t;

			time (&tm);
			t = ctime (&tm);
			t[strlen(t) - 1] = '\0';

			fp = CHK_fopen (ARG_FILENAME, "w");
		
			CHK_fprintf (fp, "%s%02d\";\n", template, value);
			CHK_fprintf (fp, "char CreationDate[] = \"%s\";\n", t);

			CHK_fclose (fp);
		}
	}

	if (GlobalErr)
	{
		fprintf (stdout, "Error: %s\n", GlobalErrMsg);
		result = FALSE;
	}

	return (result ? EXIT_SUCCESS : EXIT_FAILURE);
}
