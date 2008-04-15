/*
 * MAKELIB.C
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 08/13/90
 *   MODIFIED : 09/29/93
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 *		Copyright (c) 1990-2008, Echidna
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
 *		Program to split C source files into modules for 'C' libraries
 *
 * HISTORY
 *
 *
 * NOTES:
 *	Use MakeScan to create and Auto-Responce Type file for MakeLib to
 *  get ALL filenames of library source files.  MakeLib reads and splits
 *  and compiles all files creating a new 'makefile' in the format MakeScan
 *  expects listing all the files used in the library.  Then MakeScan is
 *  called again to create an Auto-Responce file for the Linker.
 *
 *  How does MakeLib choose to recompile a lib?
 *  Could MakeLib also archive LIB Modules??!?!?
 *
 *  MakeLib Archives LIB modules and compares Archive Date with source Date
 *  If Source is newer split file and compile...
 *  If Archive is newer un-archive old object...
 *
 *	Speed it up by reading text file once and keeping track of headers with
 *	ftell()/fseek().
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <echidna/eio.h>
#include <echidna/argparse.h>
#include <echidna/eerrors.h>
#include <echidna/strings.h>
#include <echidna/listapi.h>

/**************************** C O N S T A N T S ***************************/

#define	GLOBAL_HEADER	"/**# GLOBAL"
#define GLOBAL_SIZE		11
#define MODULE_HEADER	"/**# MODULE"
#define MODULE_SIZE		11

#define MAX_LINE_SIZE	1024

#define	ARCHIVE_EXT		".ach"
#define	OBJ_EXT			".obj"

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

extern char Version[];

char Line[MAX_LINE_SIZE];
char CMD[MAX_LINE_SIZE];

LST_LIST	 ModuleListX;
LST_LIST	*ModuleList = &ModuleListX;

short		 DebugFlag = FALSE;
short		 ArcFlag   = FALSE;

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/

/*********************************************************************
 *
 * Archive
 *
 * SYNOPSIS
 *		short Archive (char *filename, LST_NODE *nd)
 *
 * PURPOSE
 *		Archive files from current node to end of list.
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
#define MAX_BUFFER_SIZE	16384
short Archive (char *filename, LST_NODE *nd)
{
	long	size;
	int		infh = -1;
	int		outfh = -1;
	int		cnt;
	short	result = FALSE;
	char	ArcName[EIO_MAXPATH];
	char	ObjName[EIO_MAXPATH];
	char	Path[EIO_MAXDIR];
	char	Name[EIO_MAXFILE];
	char	*buffer = NULL;
/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "Archive";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	buffer = malloc (MAX_BUFFER_SIZE);
	if (!buffer) {
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("Couldn't allocate copy buffer");
		goto accleanup;
	}
	
	EIO_fnmerge (ArcName, NULL, filename, ARCHIVE_EXT);
	outfh = EIO_WriteOpen (ArcName);
	if (outfh == (-1)) {
		SetGlobalErr (ERR_GENERIC);
		GEprintf1 ("Couldn't create archive file '%s'\n", ArcName);
		goto accleanup;
	}

	while (!LST_EndOfList (nd)) {
		EIO_fnsplit (LST_NodeName(nd), Path, Name, NULL);
		EIO_fnmerge (ObjName, Path, Name, OBJ_EXT);
		infh = EIO_ReadOpen (ObjName);
		if (infh == (-1)) {
			SetGlobalErr (ERR_GENERIC);
			GEprintf1 ("Couldn't open object file '%s'\n", ObjName);
			goto accleanup;
		}
		size = EIO_FileLength (infh);
		if (size == (-1)) {
			SetGlobalErr (ERR_GENERIC);
			GEprintf1 ("Couldn't get length of file '%s'\n", ObjName);
			goto accleanup;
		}
		if (EIO_Write (outfh, &size, sizeof (size)) == (-1)) {
			SetGlobalErr (ERR_GENERIC);
			GEprintf1 ("Couldn't write length to file '%s'\n", ArcName);
			goto accleanup;
		}
		while ((cnt = (int)EIO_Read (infh, buffer, MAX_BUFFER_SIZE)) > 0) {
			if (EIO_Write (outfh, buffer, cnt) == (-1)) {
				SetGlobalErr (ERR_GENERIC);
				GEprintf1 ("Error writing to file '%s'\n", ArcName);
				goto accleanup;
			}
		}
		if (cnt < 0) {
			SetGlobalErr (ERR_GENERIC);
			GEprintf1 ("Error reading file '%s'\n", ObjName);
			goto accleanup;
		}
		EIO_Close (infh);	infh = -1;
		nd = LST_Next (nd);
	}
	result = TRUE;

accleanup:
	if (infh != (-1))	EIO_Close (infh);
	if (outfh != (-1))	EIO_Close (outfh);
	if (!result)		remove (ArcName);
	if (buffer)			free (buffer);
	return result;

} /* Archive */

/*********************************************************************
 *
 * Unarchive
 *
 * SYNOPSIS
 *		short Unarchive (char *filename)
 *
 * PURPOSE
 *		Unarchive files and add to ModuleList
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
short Unarchive (char *filename)
{
	LST_NODE	*nd;
	int			 infh = -1;
	int			 outfh = -1;
	int			 cnt;
	short		 result = FALSE;
	long		 size;
	char		*buffer = NULL;
	char		 TmpName[EIO_MAXPATH];
/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "Unarchive";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	infh = EIO_ReadOpen (filename);
	if (infh == (-1)) {
		SetGlobalErr (ERR_GENERIC);
		GEprintf1 ("Couldn't open archive '%s'", filename);
		goto uacleanup;
	}

	while ((cnt = (int)EIO_Read (infh, &size, sizeof (size))) == sizeof (size)) {
		EIO_GetTempFileName (TmpName);
		nd = LST_CreateNode (sizeof (LST_NODE), TmpName);
		if (!nd) {
			SetGlobalErr (ERR_OUT_OF_MEMORY);
			GEprintf ("OOM LST_NODE");
			goto uacleanup;
		}
		LST_AddTail (ModuleList, nd);
		strcat (TmpName, OBJ_EXT);
		outfh = EIO_WriteOpen (TmpName);
		if (outfh == (-1)) {
			SetGlobalErr (ERR_GENERIC);
			GEprintf1 ("Couldn't open object file '%s'", TmpName);
			goto uacleanup;
		}
		buffer = malloc (MAX_BUFFER_SIZE);
		if (!buffer) {
			SetGlobalErr (ERR_OUT_OF_MEMORY);
			GEprintf ("OOM Copy Buffer");
			goto uacleanup;
		}
		cnt = 1;
		while (size && cnt) {
			cnt = (int)EIO_Read (infh, buffer, MAX_BUFFER_SIZE);
			if (cnt < 0) {
				SetGlobalErr (ERR_GENERIC);
				GEprintf1 ("Error reading file '%s'", filename);
				goto uacleanup;
			}
			if (EIO_Write (outfh, buffer, cnt) == (-1)) {
				SetGlobalErr (ERR_GENERIC);
				GEprintf1 ("Error writing to file '%s'", TmpName);
				goto uacleanup;
			}
			size -= cnt;
		}
		if (!size) {
			SetGlobalErr (ERR_GENERIC);
			GEprintf1 ("File size mismatch in archive '%s'", filename);
			goto uacleanup;
		}
		free (buffer);	buffer = NULL;
		EIO_Close (outfh);	outfh  = -1;
	}

	result = TRUE;

uacleanup:
	if (outfh != (-1))	EIO_Close (outfh);
	if (infh != (-1))	EIO_Close (infh);
	if (buffer)			free (buffer);
	return result;

} /* Unarchive */

/*********************************************************************
 *
 * Compile
 *
 * SYNOPSIS
 *		short Compile (char *name, char *cmdline)
 *
 * PURPOSE
 *		Compile a file!?!?!
 *		Insert 'name' into cmdline for '%s'
 *		and 'name.obj' for '%o'
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
#define MAX_CMDLINE	1024
static char NewLine[MAX_CMDLINE];
short Compile (char *sname, char *cmdline)
{
	char	ObjName[EIO_MAXPATH];
	char	Path[EIO_MAXDIR];
	char	Name[EIO_MAXFILE];
/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "Compile";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	EIO_fnsplit (sname, Path, Name, NULL);
	EIO_fnmerge (ObjName, Path, Name, OBJ_EXT);

	/*** Substitute sname for %s and ObjName for %o in CMDLINE ***/
	{
		char	*s;
		char	*d;

		s   = cmdline;
		d   = NewLine;
		while (*s) {
			if (s[0] == '%' && tolower(s[1]) == 's') {
				s += 2;
				strcpy (d, sname);
				d += strlen (sname);
			} else if (s[0] == '%' && tolower(s[1]) == 'o') {
				s += 2;
				strcpy (d, ObjName);
				d += strlen (ObjName);
			} else {
				*d++ = *s++;
			}
		}
		*d = '\0';
		EL_printf ("\t%s\n", NewLine);

	}

	{
#define MAX_CMDARGS	60
		char	*args[MAX_CMDARGS];

		if (!argify (NewLine, MAX_CMDARGS, &args[0])) {
			SetGlobalErr (ERR_GENERIC);
			GEprintf ("Too many args?");
			return FALSE;
		}

		if (!DebugFlag && EIO_SpawnProgram (args[0], &args[0])) {
			SetGlobalErr (ERR_GENERIC);
			GEprintf ("Error executing last command");
			return FALSE;
		}
		return TRUE;
	}

} /* Compile */

/*********************************************************************
 *
 * Split
 *
 * SYNOPSIS
 *		short Split (char *filename, char *cmdline)
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
short Split (char *filename, char *cmdline)
{
	short		 status  = FALSE;
	short		 routine = 1;
	short		 rcount;
	short		 putout;
	short		 linecnt;
	short		 found;
	FILE		*infp  = NULL;
	FILE		*outfp = NULL;
	LST_NODE	*nd;
	LST_NODE	*firstnd = NULL;
	char		 TmpName[EIO_MAXPATH];
	char		 TmpExt[EIO_MAXEXT];
/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "Split";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	EIO_fnsplit (filename, NULL, NULL, TmpExt);
	do {
		putout  = TRUE;
		found   = FALSE;
		linecnt = 0;
		rcount  = 0;
		infp = fopen (filename, "r");
		if (!infp) {
			SetGlobalErr (ERR_GENERIC);
			GEprintf1 ("Couldn't open source file '%s'", filename);
			goto cleanupsp;
		}
		EIO_GetTempFileName(TmpName);
		strcat (TmpName, TmpExt);
		nd = LST_CreateNode (sizeof (LST_NODE), TmpName);
		if (!nd) {
			SetGlobalErr (ERR_OUT_OF_MEMORY);
			GEprintf ("OOM LST_NODE\n");
			goto cleanupsp;
		}
		if (!firstnd) {
			firstnd = nd;
		}
		outfp = fopen (TmpName, "w");
		if (!outfp) {
			SetGlobalErr (ERR_GENERIC);
			GEprintf1 ("Couldn't open dest file '%s'\n", TmpName);
			goto cleanupsp;
		}
		while (fgets (Line, MAX_LINE_SIZE, infp)) {
			linecnt++;
			if (!strnicmp (Line, GLOBAL_HEADER, GLOBAL_SIZE)) {
				putout = TRUE;
				fprintf (outfp, "#line %d \"%s\"\n", linecnt + 1, filename);
			} else if (!strnicmp (Line, MODULE_HEADER, MODULE_SIZE)) {
				rcount++;
				if (rcount == routine) {
					putout = TRUE;
					found  = TRUE;
					if (Line[MODULE_SIZE] == ':') {
						char	*s;

						fprintf (stderr, "\tModule:");
						s = &Line[MODULE_SIZE + 1];
						while (*s && !isspace (*s)) {
							putc (*s++, stderr);
						}
						putc ('\n', stderr);
					} else {
						fprintf (stderr, "\tModule:%s%d\n", filename, routine);
					}
					fprintf (outfp, "#line %d \"%s\"\n", linecnt + 1, filename);
				} else {
					putout = FALSE;
				}
			} else if (putout) {
				if (fputs (Line, outfp) == EOF) {
					SetGlobalErr (ERR_GENERIC);
					GEprintf1 ("Error writing to file '%s'\n", TmpName);
					goto cleanupsp;
				}
			}
		}
		if (ferror (infp)) {
			SetGlobalErr (ERR_GENERIC);
			GEprintf1 ("Error reading file '%s'\n", filename);
			goto cleanupsp;
		}
		fclose (outfp); outfp = NULL;
		fclose (infp);  infp  = NULL;

		if (found) {
			LST_AddTail (ModuleList, nd);
			if (!Compile (TmpName, cmdline)) {
				goto cleanupsp;
			}
		} else {
			LST_DeleteNode (nd);
			remove (TmpName);
		}

		routine++;
	} while (found);

	if (!DebugFlag && firstnd && ArcFlag) {
		char	Path[EIO_MAXDIR];
		char	Name[EIO_MAXFILE];

		EIO_fnsplit (filename, Path, Name, NULL);
		EIO_fnmerge (TmpName, Path, Name, NULL);
		status = Archive (TmpName, firstnd);
	} else {
		status = TRUE;
	}

cleanupsp:
	if (outfp)	fclose (outfp);
	if (infp)	fclose (infp);
	return status;

} /* Split */

/******************************** TEMPLATE ********************************/
#define ARG_ARFNAME		(newargs[0])
#define ARG_FILENAME	(newargs[1])
#define ARG_CMDLINE		(newargs[2])
#define ARG_DEBUGFLAG	(newargs[3])
#define ARG_FILELIST	(newargs[4])

char Usage[] = "Usage: MakeLib ARFNAME [FILENAME] [CMDLINE] [switches]\n";

ArgSpec Template[] = {
{	STANDARD_ARG|REQUIRED_ARG,	"ARFNAME",		"\tARFNAME  = Auto-Responce File\n", },
{	STANDARD_ARG,				"FILENAME",		"\tFILENAME = Source file to split\n", },
{	STANDARD_ARG,				"CMDLINE",		"\tCMDLINE  = Command line for compiler\n", },
{	CHRSWITCH_ARG,				"D",			"\t-D       = Debug (don't execute command)\n", },
{	CHRKEYWORD_ARG,				"F",			"\t-F<file>	= File/CMDLine List\n", },
{	0, NULL, NULL, },
};

/********************************* M A I N ********************************/
int main (
	int argc,
	char **argv
) {
	char		**newargs = NULL;
	short		 status = TRUE;
	short		 namecnt = 0;
	short		 unarc;
	FILE		*namefp;
	char		 WorkName[EIO_MAXPATH];
	char		 SourceName[EIO_MAXPATH];
	char		 SPath[EIO_MAXDIR];
	char		 SName[EIO_MAXFILE];
	char		 SExt[EIO_MAXEXT];

	fprintf (stderr, "MakeLib %s (c) Echidna 1990\n", Version);
	LST_InitList (ModuleList);

	if (!(argc == 2 && argv[1][0] == '?' && argv[1][1] == '\0')) {
		newargs = argparse (argc, argv, Template);
	}
	if (!newargs) {
		if (GlobalErrMsg) {
			fprintf (stderr, "%s\n", GlobalErrMsg);
			GlobalErrMsg = NULL;
		}
		printarghelp (Usage, Template);
		status = FALSE;
	} else {
		if (ARG_DEBUGFLAG) DebugFlag = (SWITCH_VALUE(ARG_DEBUGFLAG));

		if (ARG_FILELIST) {
			namefp = fopen (ARG_FILELIST, "r");
			if (!namefp) {
				fprintf (stderr, "Couldn't open name file '%s'\n", ARG_FILELIST);
				status = FALSE;
			} else {
#define MAX_ARGS		2
#define TARG_SOURCENAME	(args[0])
#define TARG_CMDLINE		(args[1])
				char			*args[MAX_ARGS];
				FileDateType	 sfdt;
				FileDateType	 dfdt;

				while (fgets (SourceName, MAX_LINE_SIZE, namefp)) {
					namecnt++;
					SourceName[strlen (SourceName) - 1] = '\0';
					if (argify (SourceName, MAX_ARGS, args) != MAX_ARGS) {
						fprintf (stderr, "Wrong number of args in line '%d:%s'\n", namecnt, SourceName);
						status = FALSE;
						break;
					}
					EIO_fnsplit (TARG_SOURCENAME, SPath, SName, SExt);
					EIO_fnmerge (WorkName, SPath, SName, ARCHIVE_EXT);
					unarc = FALSE;
					if (EIO_GetFileDate (WorkName, &dfdt)) {
						if (!EIO_GetFileDate (TARG_SOURCENAME, &sfdt)) {
							fprintf (stderr, "Couldn't get date for file '%s'\n", TARG_SOURCENAME);
							status = FALSE;
							break;
						}
						if (EIO_CmpDates (&sfdt, &dfdt) < 0) {
							unarc = TRUE;
						}
					}
					if (unarc) {
						if (!Unarchive (WorkName)) {
							status = FALSE;
							break;
						}
					} else {
						if (!Split (TARG_SOURCENAME, TARG_CMDLINE)) {
							status = FALSE;
							break;
						}
					}
				}
				if (ferror (namefp)) {
					fprintf (stderr, "Error reading name file '%s'\n", ARG_FILELIST);
					status = FALSE;
				}
			}
		} else {
			status = Split (ARG_FILENAME, ARG_CMDLINE);
		}

		if (status) {
			FILE	*fp;

			fp = fopen (ARG_ARFNAME, "w");
			if (!fp) {
				fprintf (stderr, "Couldn't open arffile '%s'\n", ARG_ARFNAME);
				status = FALSE;
			} else {
				LST_NODE	*nd;
				char		 Path[EIO_MAXDIR];
				char		 Name[EIO_MAXFILE];

				fprintf (fp, "MODULES = \\\n");
				while ((nd = LST_RemHead (ModuleList)) != NULL) {
					fprintf (stderr, "Deleting %s\r", LST_NodeName(nd));
					EIO_fnsplit (LST_NodeName(nd), Path, Name, NULL);
					EIO_fnmerge (WorkName, Path, Name, OBJ_EXT);
					fprintf (fp, "\t%s\t\\\n", WorkName);
				}
				fprintf (fp, "\n");
				fprintf (stderr, "\n");
				fclose (fp);
			}
		}
	}

	if (GlobalErrMsg) {
		fprintf (stderr, "%s\n", GlobalErrMsg);
	}

	return (status ? EXIT_SUCCESS : EXIT_FAILURE);
}

