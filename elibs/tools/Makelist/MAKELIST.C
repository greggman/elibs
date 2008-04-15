/*************************************************************************
 *                                                                       *
 *                              MAKELIST.C                               *
 *                                                                       *
 *************************************************************************
 
		Copyright (c) 1997-2008, Echidna

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
		Make list based on another?!?!  Originally code by Echidna 
 
   PROGRAMMERS
		Gregg A. Tavares
 
   FUNCTIONS
 
   TABS : 5
 
   HISTORY
		04/11/97 GAT: Created.

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include <echidna\ensure.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <echidna/argparse.h>
#include <echidna/listapi.h>
#include <echidna/eerrors.h>
#include <echidna/checkglu.h>
#include <echidna/strings.h>

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/

typedef struct
{
	LST_NODE	 node;
	int			 used;
	long		 line;
}
WordNode;

/****************************** G L O B A L S *****************************/

LST_LIST	 wordListX;
LST_LIST	*wordList = &wordListX;

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/***************************** T E M P L A T E ****************************/

#define	ARG_SPECFILE	(newargs[ 0])
#define ARG_SUBST		(newargs[ 1])
#define ARG_INFILE		(newargs[ 2])
#define ARG_OUTFILE		(newargs[ 3])

char Usage[] =	"Usage: MAKELIST SPECFILE SUBST INFILE OUTFILE\n"
				"\n"
				"\tReads words in INFILE and then goes through\n"
				"\tSPECFILE.  Each word from SPECFILE that matches\n"
				"\ta word in INFILE gets written to OUTFILE.  Words\n"
				"\tthat don't match are written as SUBST\n"
				"\n"
				;

ArgSpec Template[] = {
{STANDARD_ARG|REQUIRED_ARG,				"SPECFILE",	"\tSPECFILE = Source Words\n", },
{STANDARD_ARG|REQUIRED_ARG,				"SUBST",	"\tSUBST    = Substitution Word\n", },
{STANDARD_ARG|REQUIRED_ARG,				"INFILE",	"\tINFILE   = Words File to include\n", },
{STANDARD_ARG|REQUIRED_ARG,				"OUTFILE",	"\tOUTFILE  = File to write words to\n", },
{0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
{
	char			**newargs;
	int				result = TRUE;

	newargs = argparse (argc, argv, Template);

	if (!newargs)
	{
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		return EXIT_FAILURE;
 	}
	else
	{
		char	 line[256];
		FILE	*inFp;
		FILE	*outFp;
		long	 lineNo = 0;

		LST_InitList (wordList);

		inFp = CHK_fopen (ARG_INFILE, "r");

		while (fgets (line, 256, inFp))
		{
			char	*args[50];
			char	**argv;
			short	 argc;

			lineNo++;

			StripComment (line);

			argv = &args[0];
			argc = argify (line, 50, args);
			while (argc)
			{
				WordNode	*nd;

				nd = CHK_CreateNode (sizeof (WordNode), *argv, "Word Node");
				nd->line = lineNo;

				LST_AddTail (wordList, nd);

				argv++;
				argc--;
			}
		}

		CHK_fclose (inFp);

		inFp  = CHK_fopen (ARG_SPECFILE, "r");
		outFp = CHK_fopen (ARG_OUTFILE, "w");

		while (fgets (line, 256, inFp))
		{
			char	*s;
			char	*p;
			size_t	 len;

			StripComment (line);
			s = TrimWhiteSpace (line);

			//
			// find first space
			//
			p = s;
			while (*p && !isspace (*p))
			{
				p++;
			}
// EL_printf ("line = '%s'\n", s);
			len = p - s;
			if (len && *p)
			{
				//
				// mark end of first word
				//
				*p = '\0';
				p++;

				//
				// find start of second word
				//
				while (*p && isspace (*p))
				{
					p++;
				}

// EL_printf ("Found word '%s'\n", s);

				{
					WordNode	*nd;

					nd = (WordNode*)LST_FindIName (wordList, s);
					if (nd)
					{
// EL_printf ("printing '%s'\n", p);
						nd->used = TRUE;
						CHK_fprintf (outFp, "%s\n", p);
					}
					else
					{
// EL_printf ("printing '%s'\n", ARG_SUBST);
						CHK_fprintf (outFp, "%s\n", ARG_SUBST);
					}
				}
			}
		}

		//
		// print errors for words not found
		//
		{
			WordNode	*nd;

			nd = (WordNode*)LST_Head (wordList);
			while (!LST_EndOfList (nd))
			{
				if (!nd->used)
				{
					ErrMess ("ERROR: Word '%s' not found in line #%ld\n", LST_NodeName(nd), nd->line);
				}
				nd = (WordNode*)LST_Next (nd);
			}
		}
	}


	if (ErrorCount) {
		CHK_fprintf (stdout, "Errors: %d\n", ErrorCount);
		result = FALSE;
	}

	if (WarnCount) {
		CHK_fprintf (stdout, "Warnings: %d\n", WarnCount);
	}

	if (GlobalErr) {
		CHK_fprintf (stdout, "Error: %s\n", GlobalErrMsg);
		result = FALSE;
	}

	return (result ? EXIT_SUCCESS : EXIT_FAILURE);
}

