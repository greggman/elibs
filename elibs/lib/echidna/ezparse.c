/*
 * EZPARSE.C
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 03/07/91
 *   MODIFIED : 05/13/94
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 *		Copyright (c) 1996-2008, Echidna
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
 *
 * DESCRIPTION
 *		Routines to simply parse a file!?!?!
 *
 * HISTORY
 *
 *		02/28/94 Monday (GAT) - Added EZParseLineNo
 *	
 *		02/28/94 Monday (GAT) - Changed GetLine to EZParseGetLine
 *	
 *	
 *	
*/

#include "platform.h"
#include "switches.h"

#include <stdio.h>
#include <stdarg.h>

#include "echidna/strings.h"
#include "echidna/ezparse.h"
#include "echidna/argparse.h"
#include "echidna/eerrors.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/

typedef struct
{
	short	 KeepBlanks;
	long	 LineNo;
	char	*FileName;
}
EZParseTracker;

/****************************** G L O B A L S *****************************/

short KeepBlanks;
static EZParseTracker	 badezpt =
{
	FALSE, 0, "Popped one to many EZParse Files\n",
};
static EZParseTracker	*ezpt = &badezpt;

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/

short EZParseLineNo (void)	{ return (short)ezpt->LineNo; }
void EZParseKeepBlankLines (short flag) { KeepBlanks = flag; }

/*********************************************************************
 *
 * EZParseError
 *
 * SYNOPSIS
 *		void  EZParseError (char *fmt, ...)
 *
 * PURPOSE
 *		print an error message with filename and lineno.
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
void EZParseError (char *fmt, ...)
{
	va_list ap;

	++ErrorCount;
	fprintf (stdout, "Error %s %ld:", ezpt->FileName, ezpt->LineNo);

	va_start (ap,fmt); /* make ap point to 1st unnamed arg */
	vfprintf (stdout, fmt,ap);
	va_end (ap);	/* clean up when done */

	fprintf (stdout, "\n");
}

/*********************************************************************
 *
 * EZParseError1
 *
 * SYNOPSIS
 *		void EZParseError1 (char *msg1, char *msg2)
 *
 * PURPOSE
 *		print an error message with filename and lineno.
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
void EZParseError1 (char *msg1, char *msg2)
{
	++ErrorCount;
	fprintf (stdout, "Error %s %ld:%s %s\n", ezpt->FileName, ezpt->LineNo, msg1, msg2);

} /* EZParseError1 */

/**************************************************************************
 *
 * EZParseGetLine 
 *
 * SYNOPSIS
 *		short EZParseGetLine (FILE *fp, char *linebuff, short maxline)
 *
 * PURPOSE
 *		Get a line from specfied file.  Strip ';' comments.
 *		Skip blank lines and lines that start with '*' or '#'.
 *		Also remove end-of-line.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		> 0	= okay.
 *		= 0 = End of File.
 *		< 0 = error.
 *
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
short EZParseGetLine (FILE *fp, char *linebuff, short maxline, short strip)
{
	char	*s;
	char	*d;
	char	*newline;
	short	 len;

	len     = 0;
	newline = linebuff;
	while (fgets (newline, maxline - len, fp)) {
		ezpt->LineNo++;
		s   = linebuff;
		len = strlen (s);
		if (len) {
			if (len > 1 && s[len-2] == '\\') {
				s[len-2] = '\0';
				newline  = s+len-2;
			} else {
				if (strip) {
					StripComment (s);
				}
				d = TrimWhiteSpace (s);
				if (strip) {
					s = d;
				}
				if ((ezpt->KeepBlanks || strlen (s)) && ((*s != '*' && *s != '#') || !strip)) {
					d = linebuff;
					while (*s) {
						*d++ = *s++;
					}
					*d = '\0';
					return 1;
				}
				len     = 0;
				newline = linebuff;
			}
		}
	}
	if (feof (fp)) {
		return 0;
	}
	return (-1);

} /* EZParseGetLine  */

/*********************************************************************
 *
 * EZResetParseFilename
 *
 * SYNOPSIS
 *		void EZResetParseFilename (char *filename);
 *
 * PURPOSE
 *		Sets filename for error messages and resets line number
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
void EZResetParseFilename (char *filename)
{
	ezpt->FileName   = filename;
	ezpt->LineNo     = 0;
	ezpt->KeepBlanks = KeepBlanks;

} /* EZResetParseFilename */

/*********************************************************************
 *
 * EZParseFile
 *
 * SYNOPSIS
 *		short EZParseFile (KeyWord *kw, char *filename, void *userdata)
 *
 * PURPOSE
 *		Parse a simple text file.
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
 *
 * TODO
 *  * Use the pre-processor scanner!
 *
*/
short EZParseFile (KeyWord *keywords, char *filename, short maxargs, void *userdata)
{
	EZParseTracker	 ez;
	EZParseTracker	*oldezpt = ezpt;
	char	 line[MAX_PARSE_LINE];
	FILE	*fp			= NULL;
	KeyWord	*kw;
	char	**args		= NULL;
	char	**newargs;
	char	**junkargs;
	short	 status 	= FALSE;
	short	 result;
	short	 error  	= FALSE;
	short	 count;
	int		 junkcount;

	memset (&ez, 0, sizeof (ez));
	ezpt = &ez;

	EZResetParseFilename (filename);

	fp = fopen (filename, "r");
	if (!fp) {
		SetGlobalErr (ERR_FILE_NOT_FOUND);
		GEcatf1 ("\nCouldn't open '%s' for parsing", filename);
		goto pfcleanup;
	}

	args = malloc ((maxargs + 1) * sizeof (char *));
	if (!args) {
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEcatf1 ("\nOOM: args EZParseFile(%s)", filename);
		goto pfcleanup;
	}

	for (;;) {
		result = EZParseGetLine (fp, line, MAX_PARSE_LINE, TRUE);
		if (result <= 0) {
			break;
		}
		count = argify (line, maxargs, args);
		kw    = keywords;
		while (kw->Keyword) {
			if (!stricmp (kw->Keyword, args[0])) {
				junkcount = count - 1;
				junkargs  = &args[1];
				newargs   = argvark (&junkcount, &junkargs, kw->Template, FALSE);
				if (!newargs) {
					EZParseError1 (GlobalErrMsg, "");
					ClearGlobalError ();
					error = TRUE;
				} else {
					result = kw->ParseFunc (newargs, userdata);
					if (result <= 0) {
						error = TRUE;
						if (result < 0) {
							EZParseError1 ("Fatal Error:", "");
							goto pfcleanup;
						}
					}						
				}
				break;
			}
			kw++;
		}
		if (!kw->Keyword) {
			EZParseError1 ("Unknown keyword", args[0]);
			error = TRUE;
		}
	}
	if (!error && !result) {
		status = TRUE;
	} else {
		SetGlobalErr (ERR_GENERIC);
		GEcatf1 ("\nError parsing '%s'", filename);
	}

pfcleanup:
	if (args)	free (args);
	if (fp)		fclose (fp);
	ezpt = oldezpt;
	return status;

} /* EZParseFile */

