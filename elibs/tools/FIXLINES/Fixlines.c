/*
 * FIXLINES.C
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 05/09/93
 *   MODIFIED : 08/22/94
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
 *		Converts an ASCII text file's end-of-line markers
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

#define MAXLINE		4096

/******************************** T Y P E S *******************************/

/****************************** G L O B A L S *****************************/

char			 InLine[MAXLINE];
short			 Mode;

char			 EndOfLine[][4] =
{
	{ 0x00, },
	{ 0x0D, 0x0A, 0x00, },
	{ 0x0D, 0x00, },
	{ 0x0A, 0x00, },
};
	
/******************************* M A C R O S ******************************/

/******************************* T A B L E S ******************************/

/***************************** R O U T I N E S ****************************/

short getline (char *s, short maxlen, FILE *fp)
{
	int		c;
	short 	count = 0;

	for (;;)
	{
		maxlen--;
		if (!maxlen)
		{
			*s = '\0';
			return TRUE;
		}
		
		c = getc(fp);
		if (c == EOF)
		{
			if (ferror (fp))
			{
				EL_printf ("Error reading file\n");
				exit (EXIT_FAILURE);
			}
			*s = '\0';
			if (count == 0)
			{
				return FALSE;
			}
			else
			{
				return TRUE;
			}
		}

		if (c == 13)
		{
			c = getc (fp);
			if (c != 10)
			{
				ungetc (c, fp);
			}
			*s = '\0';
			return TRUE;
		}
		if (c == 10)
		{
			*s = '\0';
			return TRUE;
		}

		*s++ = c;
		count++;
	}
}

/******************************** TEMPLATE ********************************/

#define ARG_INFILE		(newargs[ 0])
#define ARG_OUTFILE		(newargs[ 1])
#define	ARG_MODE		(newargs[ 2])

char Usage[] =	"Usage: FIXLINES INFILE OUTFILE [options]\n"
				"\tTranslates end-of-lines but keeps date intact\n";

ArgSpec Template[] = {
{STANDARD_ARG|REQUIRED_ARG,				"INFILE",	"\tINFILE  = ASCII File to read\n", },
{STANDARD_ARG|REQUIRED_ARG,				"OUTFILE",	"\tOUTFILE = ASCII File to write\n", },
{CHRKEYWORD_ARG,						"M",		"\t-M<mode>= End of line mode\n"
													"\t    0 = Native (default)\n"
													"\t    1 = MS-DOS (cr/lf)\n"
													"\t    2 = Mac (cr)\n"
													"\t    3 = Unix (lf)\n", },
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
		short	 lineno = 0;

		if (ARG_MODE)	Mode=EL_atos(ARG_MODE);

		if (!Mode)
		{
			#if _EL_PLAT_WIN32__
				Mode = 1;
			#elif _EL_PLAT_SGI__
				Mode = 2;
			#elif _EL_PLAT_MACOS__
				Mode = 3;
			#else
			#error Need Code
			#endif
		}

		infp = fopen (ARG_INFILE, "rb");
		if (!infp)
		{
			EL_printf ("Couldn't open '%s'\n", ARG_INFILE);
			exit (EXIT_FAILURE);
		}

		outfp = fopen (ARG_OUTFILE, "wb");
		if (!outfp)
		{
			EL_printf ("Couldn't open '%s'\n", ARG_OUTFILE);
			exit (EXIT_FAILURE);
		}

		while (getline (InLine, MAXLINE, infp))
		{
			lineno++;
			fputs (InLine, outfp);
			fputs (&EndOfLine[Mode][0], outfp);
		}
		fclose (infp);
		fclose (outfp);

		{
			FileDateType	fdt;

			if (!EIO_GetFileDate (ARG_INFILE, &fdt)) {
				goto cleanup;
			}

			if (!EIO_SetFileDate (ARG_OUTFILE, &fdt)) {
				goto cleanup;
			}
		}

		{
			FileAttribType	fat;

			if (!EIO_GetFileAttrib (ARG_INFILE, &fat)) {
				goto cleanup;
			}

			if (!EIO_SetFileAttrib (ARG_OUTFILE, &fat)) {
				goto cleanup;
			}

		}

		{
			FileCommentType	fct;

			if (!EIO_GetFileComment (ARG_INFILE, &fct)) {
				goto cleanup;
			}

			if (!EIO_SetFileComment (ARG_OUTFILE, &fct)) {
				goto cleanup;
			}
		}
	}

cleanup:
	if (GlobalErr) {
		EL_printf ("%s\n", GlobalErrMsg);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

