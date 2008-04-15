/*
 * CHANGE.C
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 05/09/93
 *   MODIFIED : 11/02/93
 *       TABS : 05 09
 *
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 *		Copyright (c) 1993-2008, Echidna
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
 *		Takes an file and changes the text (search and replace)
 *		based on a responce file
 *
 * HISTORY
 *	
 * TODO
 *
*/

#include "platform.h"
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

/**************************** C O N S T A N T S ***************************/

#define MAXLINE		1024

/******************************** T Y P E S *******************************/

/****************************** G L O B A L S *****************************/

short			 Verbose      = FALSE;
short			 CaseSensitive= FALSE;

char			 InLine[MAXLINE];
char			 OutLine[MAXLINE];

/******************************* M A C R O S ******************************/

/******************************* T A B L E S ******************************/

/***************************** R O U T I N E S ****************************/

/******************************** TEMPLATE ********************************/

#define ARG_INFILE			(newargs[ 0])
#define ARG_OUTFILE			(newargs[ 1])
#define ARG_VERBOSE			(newargs[ 2])
#define ARG_CASESENSITIVE	(newargs[ 3])
#define ARG_REPLACE			(newargs[ 4])

char Usage[] = "Usage: CHANGE INFILE OUTFILE [S=R S=R ...]\n";

ArgSpec Template[] = {
{STANDARD_ARG|REQUIRED_ARG,				"INFILE",	"\tINFILE  = ASCII File to read\n", },
{STANDARD_ARG|REQUIRED_ARG,				"OUTFILE",	"\tOUTFILE = ASCII File to write\n", },
{CHRSWITCH_ARG,							"V",		"\t-V      = Verbose (show errors)\n", },
{CHRSWITCH_ARG,							"C",		"\t-C      = Case Sensitive\n", },
{STANDARD_ARG|REQUIRED_ARG|MULTI_ARG,	"REPLACE",	"\tREPLACE = String pairs to search and replace\n"
													"            Example /jim/james/\n", },
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

		FILE	*infp;
		FILE	*outfp;

		Verbose       = SWITCH_VALUE(ARG_VERBOSE);
		CaseSensitive = SWITCH_VALUE(ARG_CASESENSITIVE);

		infp = fopen (ARG_INFILE, "r");
		if (!infp)
		{
			EL_printf ("Couldn't open '%s'\n", ARG_INFILE);
			exit (EXIT_FAILURE);
		}

		outfp = fopen (ARG_OUTFILE, "w");
		if (!outfp)
		{
			EL_printf ("Couldn't open '%s'\n", ARG_INFILE);
			exit (EXIT_FAILURE);
		}


		while (fgets (InLine, MAXLINE, infp))
		{
			char	*s    = InLine;
			char	*d	  = OutLine;

			while (*s)
			{
				char	**replace;
				short	 rep = FALSE;

				replace = MULTI_ARGLIST(ARG_REPLACE);
			
				while (*replace && !rep)
				{
					char	 delim;
					char	*r;
					char	*c;

					r     = *replace;
					c     = s;
					delim = *r++;

					if (*r)
					{
						while (*r && *r != delim && *c && (CaseSensitive ? (*c == *r) : (toupper(*c) == toupper(*r))))
						{
							r++;
							c++;
						}

						if (*r == delim)
						{
							s = c;
							r++;

							while (*r && *r != delim)
							{
								*d++ = *r++;
								rep  = TRUE;
							}
						}
					}

					replace++;
				}
				
				if (!rep)
				{
					*d++ = *s++;
				}

			}

			*d = '\0';

			fputs (OutLine, outfp);

		}

	}

	if (GlobalErr) {
		EL_printf ("%s\n", GlobalErrMsg);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

