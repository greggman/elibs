/*
 * EGGSFILT.C
 *
 *  COPYRIGHT : 1993 Echidna.
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 05/09/93
 *   MODIFIED : 10/23/94
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

#include <echidna/platform.h>
#include "switches.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <echidna/eerrors.h>
#include <echidna/argparse.h>
#include <echidna/strings.h>
#include <echidna/eio.h>

/**************************** C O N S T A N T S ***************************/

#define MAXLINE		1024

/******************************** T Y P E S *******************************/

/****************************** G L O B A L S *****************************/

short			 Verbose      = FALSE;

char			 InLine[MAXLINE];

/******************************* M A C R O S ******************************/

/******************************* T A B L E S ******************************/

/***************************** R O U T I N E S ****************************/

char *SkipWhiteSpace (char *s)
{
	while (isspace (*s))
	{
		s++;
	}
	return s;
}

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

		*s++ = (char)c;
		count++;
	}
}

/******************************** TEMPLATE ********************************/

#define ARG_INFILE		(newargs[ 0])
#define ARG_OUTFILE		(newargs[ 1])
#define ARG_VERBOSE		(newargs[ 2])
#define ARG_MODE		(newargs[ 3])

char Usage[] =	"Usage: echidnaFILT INFILE OUTFILE\n"
				"\n"
				"\tTranslates 'C' include statments\n"
				"\tto local platform.\n"
				"\n"
				;

ArgSpec Template[] = {
{STANDARD_ARG|REQUIRED_ARG,				"INFILE",	"\tINFILE  = ASCII File to read\n", },
{STANDARD_ARG|REQUIRED_ARG,				"OUTFILE",	"\tOUTFILE = ASCII File to write\n", },
{CHRSWITCH_ARG,							"V",		"\t-V      = Verbose (show errors)\n", },
{CHRKEYWORD_ARG,						"M",		"\t-M<mode>= 0 native\n"
													"\t          1 MS-DOS\n"
													"\t          2 Mac\n"
													"\t          3 Unix\n"
													"\t          4 3DO\n", },
{0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
{
	char			**newargs;
	char			 destDirSep = DIRSEP;

	#if _EL_PLAT_WIN32__
	char			*destEOL    = "\x0d\x0a";
	#elif _EL_PLAT_MACOS__
	char			*destEOL    = "\x0d";
	#elif _EL_PLAT_SGI__
	char			*destEOL    = "\x0a";
	#endif

	newargs = argparse (argc, argv, Template);

	if (!newargs) {
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		return EXIT_FAILURE;
	} else {

		FILE	*infp;
		FILE	*outfp;
		short	 lineno = 0;

		Verbose  =  SWITCH_VALUE(ARG_VERBOSE);

		if (ARG_MODE)
		{
			long	mode;

			mode = EL_atol (ARG_MODE);
			switch (mode)
			{
			case 0:	// native
				break;
			case 1: // ms-dos
				destDirSep = '\\';
				destEOL    = "\x0d\x0a";
				break;
			case 2: // mac
				destDirSep = ':';
				destEOL    = "\x0d";
				break;
			case 3: // unix
				destDirSep = '/';
				destEOL    = "\x0a";
				break;
			case 4: // 3do
				destDirSep = '/';
				destEOL    = "\x0d";
				break;
			default:
				EL_printf ("Error: Unknown mode #%ld\n", mode);
				return EXIT_FAILURE;
			}
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
			char	*s;
			char	 quote;

			lineno++;

			s = SkipWhiteSpace (InLine);
			while (isspace (*s))
			{
				s++;
			}

			if (!strncmp (s, "#include", 8))
			{
				//
				// found include line, translate it
				//
				s += 8;
				s  = SkipWhiteSpace (s);

				if (Verbose)
				{
					EL_printf ("Old:#%4d:%-60s\n", lineno, InLine);
				}

				switch (*s)
				{
				case '<':
					quote = '>';
					break;
				case '"':
					quote = '"';
					break;
				default:
					EL_printf ("Error1: Malformed Include at line %d in file %s\n", lineno, ARG_INFILE);
					exit (EXIT_FAILURE);
				}

				s++;

				while (*s && *s != quote)
				{
					if (
						(*s == '\\') ||
						(*s == '/')  ||
						(*s == ':')
					   )
					{
						*s = destDirSep;
					}
					s++;
				}

				if (*s != quote)
				{
					EL_printf ("Error2: Malformed Include at line %d in file %s\n", lineno, ARG_INFILE);
					exit (EXIT_FAILURE);
				}

				if (Verbose)
				{
					EL_printf ("New:#%4d:%-60s\n", lineno, InLine);
				}

			}

			fputs (InLine, outfp);
			fputs (destEOL, outfp);
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

