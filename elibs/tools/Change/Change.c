/*
 * CHANGE.C
 *
 *  COPYRIGHT : 1993 Echidna.
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

