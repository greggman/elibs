/*************************************************************************
 *                                                                       *
 *                               READINI.C                               *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION
		Reads .INI file type files

		optionally supports the following pre-processor like directives
		
	    #include "filename"   # include a file (like C #include)
	    #define var=namename "value" # defines a var (all ENV vars are included)
		
		comments start with ; like assembly language
		
		you can add include paths with
			EIO_AddIncludePath (EIO_INCPATH_INIS, pathtoadd);

   PROGRAMMERS


   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/11/96 GAT: Created from code by Echidna
		06/20/01 GAT: made it so FindSection adds the [] if they are not included
		06/20/01 GAT: made it so FindNextINILine, if no = is part of the search, then looks for
		              search\s*= and skips whitespace following the = (think perl regular expressions)
		05/08/02 GAT: added #define and #include


 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include <string.h>
#include <stdio.h>

#include "echidna/strings.h"
#include "echidna/listapi.h"
#include "echidna/readini.h"
#include "echidna/eerrors.h"
#include "echidna/eio.h"

/**************************** C O N S T A N T S ***************************/

#define MAX_LINE_SIZE	4096

/*------------------------------------------------------------------------*/
/**# MODULE:READINI_readIni                                               */
/*------------------------------------------------------------------------*/

/****************************** G L O B A L S *****************************/

uint16 uwWarnErrCount;

static BOOL fCaseSensitive = TRUE;
static BOOL fSaveBlankLines = FALSE;
static BOOL fTrimWhiteSpace = TRUE;
static BOOL fStripComments = TRUE;
static BOOL fCommentAnywhere = TRUE;
static BOOL fMergeSections = TRUE;
static BOOL fExpandEVars = TRUE;
static BOOL fPreprocess  = TRUE;

static const char *szComment = ";";
static const char *szSectionMarker = "[";

static uint16 uwLineCount;
static char szLine[MAX_LINE_SIZE];


/***************************** R O U T I N E S ****************************/

void SetINICaseSensitive(BOOL f)
{
	fCaseSensitive = f;
}

void SetINISaveBlankLines(BOOL f)
{
	fSaveBlankLines = f;
}

void SetINITrimWhiteSpace(BOOL f)
{
	fTrimWhiteSpace = f;
}

void SetINIStripComments(BOOL f)
{
	fStripComments = f;
}

void SetINICommentAnywhere(BOOL f)
{
	fCommentAnywhere = f;
}

void SetINIMergeSections(BOOL f)
{
	fMergeSections = f;
}

void SetINIComment(const char *sz)
{
	szComment = sz;
}

void SetINISectionMarker(const char *sz)
{
	szSectionMarker = sz;
}

uint16 GetINIWarnings(void)
{
	return uwWarnErrCount;
}

/*********************************************************************
 *
 * GetLine
 *
 * SYNOPSIS
 *		char *GetLine (char *line, size_t size, FILE *pFile)
 *
 * PURPOSE
 *		Read a line from a text file <pFile> into specifed buffer <line>.
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
static char *GetLine (char *line, size_t size, FILE *pFile)
{
	static char	 workLine[MAX_LINE_SIZE];
	char	*pch;

	do
	{
		while ((pch = fgets (workLine, size, pFile)) != NULL)
		{
			uint16 chCnt;
			
			ENSURE (strlen(workLine) < MAX_LINE_SIZE);

			uwLineCount++;

			if (fExpandEVars)
			{
				EIO_ExpandEVars (line, pch, size);
				ENSURE (strlen(line) < size);
			}
			else
			{
				strncpy (line, pch, size);
				line[size - 1] = '\0';
			}

			pch = line;
			if (fTrimWhiteSpace)
			{
				pch = TrimWhiteSpace(pch);
			}

			chCnt = strlen(pch);
			if (chCnt && pch[--chCnt] == '\n')
				pch[chCnt] = 0;

			if (fStripComments)
			{
				if (fCommentAnywhere)
				{
					char *pch2;

					if ((pch2 = strstr(pch, szComment)) != NULL)
					{
						*pch2 = 0;
						if (fTrimWhiteSpace)
						{
							pch = TrimWhiteSpace(pch);
						}
					}
				}
				else
				{
					if (strncmp(pch, szComment, strlen(szComment)) == 0)
						break;
				}
			}

			if (fSaveBlankLines || (!fSaveBlankLines && *pch))
				return pch;
		}
	} while (pch);
	return NULL;

} /* GetLine */

/*********************************************************************
 *
 * AddINILine
 *
 * SYNOPSIS
 *		ConfigLine * AddINILine (SectionTracker *pst, char *pszIniLine, uint16 lineNo)
 *
 * PURPOSE
 *		Add a new line to the specified section
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
ConfigLine *AddINILine (Section *pst, const char *pszIniLine, uint16 lineNo)
{
	ConfigLine	*pLine;

	if ((pLine = (ConfigLine*)LST_CreateNode (sizeof (ConfigLine), pszIniLine)) != NULL)
	{
		pLine->LineNo = lineNo;
		LST_AddTail(&pst->LineList, pLine);
	}

	return pLine;
} // AddINILine


/*********************************************************************
 *
 * AddINISection
 *
 * SYNOPSIS
 *		Section * AddINISection (IniList *pIniList, char *pszSectionName)
 *
 * PURPOSE
 *		Create a new empty section
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
Section *AddINISection (IniList *pIniList, const char *pszSectionName)
{
	Section		*pSection;

	if ((pSection = (Section*)LST_CreateNode(sizeof (Section), pszSectionName)) != NULL)
	{
		LST_AddTail(pIniList, pSection);

		LST_InitList(&pSection->LineList);
	}

	return pSection;
} // AddINISection


/*********************************************************************
 *
 * CreateINI
 *
 * SYNOPSIS
 *		IniList * CreateINI (char *name)
 *
 * PURPOSE
 *		Create a new empty INI list
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
IniList *CreateINI (const char *name)
{
	IniList	*pIniList;

	if ((pIniList = (IniList *) LST_CreateList(name)) == NULL)
	{
		ErrMess ("OOM INI list.\n");
	}

	return pIniList;
} // CreateINI

/*********************************************************************
 *
 * AppendINI
 *
 * SYNOPSIS
 *		IniList * AppendINI (IniList *pIniList, char *filename)
 *
 * PURPOSE
 *		Append initialization file and store in appropriate linked lists.
 *
 * INPUT
 *		pIniList:			previous iniList or NULL
 *		filename:			File to read and store.
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		Returns pointer to list of initialization sections,
 *		or NULL if unsuccessful.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
IniList *AppendINI(IniList *pIniList, const char *filename)
{
	char		*pch;
	FILE		*pFile;

	LST_LIST	*pCurrentList = NULL;
	Section		*pSection = NULL;

	uwLineCount = 0;
	uwWarnErrCount = 0;

	if ((pFile = fopen (filename, "r")) == NULL)
	{
		ErrMess ("Couldn't open configuration file '%s'\n", filename);
		return NULL;
	}

	if (!pIniList)
	{
		if ((pIniList = CreateINI(filename)) == NULL)
		{
			return NULL;
		}
	}

	/*** Read Config File ***/

	while ((pch = GetLine(szLine, MAX_LINE_SIZE, pFile)) != NULL)
	{
		if (strncmp (pch, szSectionMarker, strlen(szSectionMarker)) == 0)
		{
			pCurrentList = NULL;

			if (fMergeSections)
			{
				pSection = (Section*)LST_Head(pIniList);

				while (!LST_IsEOList(pSection))
				{
					if ( (fCaseSensitive &&  strcmp(szSectionName(pSection), pch) == 0) ||
						(!fCaseSensitive && stricmp(szSectionName(pSection), pch) == 0))
					{
						pCurrentList = &pSection->LineList;
						break;
					}
					pSection = (Section*)LST_Next(pSection);
				}
			}
			if (!pCurrentList)
			{
				if ((pSection = AddINISection(pIniList, pch)) == NULL)
				{
					ErrMess ("OOM file '%s', line %d.\n", filename, uwLineCount);
/**/				goto ABORT;
				}

				pCurrentList = &pSection->LineList;
			}
		}
		else if (!strnicmp (pch, "#define", 7) && isspace(pch[7]))
		{
			char* equal;
			pch = TrimWhiteSpace (pch + 7);
			
			equal = strchr (pch, '=');
			if (!equal)
			{
				ErrMess ("no '=' in #define in file '%s', line %d.\n", filename, uwLineCount);
			}
			else
			{
				putenv (pch);
			}
		}
		else if (!strnicmp (pch, "#include", 8) && isspace(pch[8]))
		{
			int mode = 0;	// '>' = #include <>, '"' = #include ""
			
			pch = TrimWhiteSpace (pch + 8);
			
			if (*pch == '<')
			{
				mode = '>';
			}
			else if (*pch == '"')
			{
				mode = '"';
			}
			else
			{
				ErrMess ("missing quotes/brackets in #include in file '%s', line %d.\n", filename, uwLineCount);
			}
			
			if (mode)
			{
				char *newfile;
				
				pch++;
				newfile = pch;
				
				while (*pch && *pch != mode) pch++;
				if (!*pch)
				{
					ErrMess ("missing quotes/brackets in #include in file '%s', line %d.\n", filename, uwLineCount);
				}
				else if (pch - newfile + 1 > EIO_MAXPATH)
				{
					ErrMess ("filename too long in #include in file '%s', line %d.\n", filename, uwLineCount);
				}
				else
				{
					int  fFound = FALSE;
					char newfilename[EIO_MAXPATH];
					char fixedfilename[EIO_MAXPATH];
					
					strncpy (newfilename, newfile, pch - newfile);
					newfilename[pch - newfile] = '\0';
					
					if (EIO_FindInclude (fixedfilename, EIO_INCPATH_INIS, newfilename, filename, TRUE))
					{
						AppendINI(pIniList, fixedfilename);
					}
					else
					{
						ErrMess ("couldn't open include file %s\n", newfilename);
					}
				}            
			}
		}
		else if (!pCurrentList)
		{
			WarnMess ("File %s, line %d: No section header.\n", filename, uwLineCount);
			uwWarnErrCount++;
		}
		else
		{
			if ((AddINILine (pSection, pch, uwLineCount)) == NULL)
			{
				ErrMess ("OOM file '%s', line %5d.\n", filename, uwLineCount);
/**/			goto ABORT;
			}
		}
	}

	fclose(pFile);
	return pIniList;

ABORT:
	FreeINI(pIniList);
	return NULL;


} // AppendINI

/*********************************************************************
 *
 * FindSection
 *
 * SYNOPSIS
 *		SectionTracker *FindSection(SectionTracker *pst,
 *										IniList *pIniList, char *szSectionName)
 *
 * PURPOSE
 *		Find 1st section that matches <szSectionName>, return
 *		pointer to list of configuration lines for that section.
 *
 * INPUT
 *		pst:					pointer to SectionTracker to fill in
 *		pIniList:			pointer to IniList of sections to search
 *		sz:					name of section to find
 *
 * AFFECTS
 *
 *
 * RETURN VALUE
 *		Returns pointer to a section tracker, or NULL if unsuccessful.
 *		Pass to GetNextINILine to get LST_Next line from section.
 *
 * HISTORY
 *
 *		06/20/01 : If '[]' is not part of the sectionName then add it
 *
 * SEE ALSO
 *
*/
SectionTracker *FindSection(SectionTracker *pst, IniList *pIniList, const char *sz)
{
	SectionTracker* pResult = NULL;
	
	if (pIniList)
	{
		Section *pSection;
		char* sectionName = NULL;
		
		// add the '[' if not passed in
		if (*sz != '[')
		{
			int len = strlen(sz);
			
			sectionName = malloc (len + 3);
			if (!sectionName)
			{
				ErrMess ("OOM FindSection\n");
				return NULL;
			}
			strcpy (sectionName + 1, sz);
			sectionName[0]       = '[';
			sectionName[1 + len] = ']';
			sectionName[2 + len] = '\0';
			sz = sectionName;
		}

		pSection = (Section*)LST_Head(pIniList);

		while (!LST_IsEOList(pSection))
		{
			if ( (fCaseSensitive &&  strcmp(szSectionName(pSection), sz) == 0) ||
				(!fCaseSensitive && stricmp(szSectionName(pSection), sz) == 0))
			{
				pst->ltSection = &pSection->LineList;
				pst->clCurrentLine = (ConfigLine *) pst->ltSection;
				pResult = pst;
				break;
			}
			pSection = (Section*)LST_Next(pSection);
		}
		
		if (sectionName) free (sectionName);
	}
	return pResult;
}


/*********************************************************************
 *
 * FindNextINILine
 *
 * SYNOPSIS
 *		char *FindNextINILine(SectionTracker *pst, char *sz)
 *
 * PURPOSE
 *		Find LST_Next config line from a section that STARTS WITH
 *		(matches) the string <sz>. Update variables to
 *		reflect current line being processed.
 *
 * INPUT
 *		pst:				pointer to SectionTracker to get lines from
 *		sz:					name of configuration to match
 *
 * AFFECTS
 *		pst->clCurrentLine is updated to point to line that starts with <sz>.
 *
 * RETURN VALUE
 *		Returns pointer to 1st character in matching string
 *		following <sz>, or NULL if no more lines.
 *
 * HISTORY
 *		06/20/01 : If = is NOT part of the string then search for name\s*= (think perl regular expressions)
 *		           then skip the whitespace after the =
 *
 *
 * SEE ALSO
 *
*/

char *FindNextINILine(SectionTracker *pst, const char *sz)
{
	ConfigLine *pLine;
	uint16 uwLen;

	uwLen = strlen(sz);

	while ((pLine = GetNextINILine(pst)) != NULL)
	{
		if (( fCaseSensitive &&  strncmp(szConfigLine(pLine), sz, uwLen) == 0) ||
			(!fCaseSensitive && strnicmp(szConfigLine(pLine), sz, uwLen) == 0))
		{
			char* s = szConfigLine(pLine)+uwLen;
			
			if (sz[uwLen - 1] != '=')
			{
				// skip whitespace
				while (*s && isspace(*s)) s++;
				// skip =
				if (*s == '=') s++;
				// skip whitespace
				while (*s && isspace(*s)) s++;
			}
			return s;
		}
	}
	return NULL;
}

/*------------------------------------------------------------------------*/
/**# MODULE:READINI_FreeINI                                               */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 *	FreeINI
 *
 * SYNOPSIS
 *		void FreeINI(IniList *pIniList)
 *
 * PURPOSE
 *		Free linked list structure generated by ReadINI.
 *
 * INPUT
 *		pIniList:			pointer to structure to free.
 *
 * AFFECTS
 *
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void FreeINI(IniList *pIniList)
{
	if (pIniList)
	{
		Section *pSection;

		while ((pSection = (Section*)LST_RemTail(pIniList)) != NULL)
		{
			LST_EmptyList(&pSection->LineList);

			LST_DeleteNode(pSection);
		}
	}
	LST_DeleteList(pIniList);
}

/*------------------------------------------------------------------------*/
/**# MODULE:READINI_PrintINI                                              */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 *	PrintINI
 *
 * SYNOPSIS
 *		void PrintINI(IniList *pIniList)
 *
 * PURPOSE
 *		Print linked list structure generated by ReadINI.
 *
 * INPUT
 *		pIniList:			pointer to structure to print.
 *
 * AFFECTS
 *
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void PrintINI(IniList *pIniList)
{
	if (pIniList)
	{
		Section *pSection;

		pSection = (Section*)LST_Head(pIniList);

		while (!LST_IsEOList(pSection))
		{
			ConfigLine	*pLine;

			printf("'%s'\n", szSectionName(pSection));

			pLine = (ConfigLine*)LST_Head(&pSection->LineList);

			while (!LST_IsEOList(pLine))
			{
				printf("  '%s'\n", szConfigLine(pLine));

				pLine = (ConfigLine*)LST_Next(pLine);
			}
			pSection = (Section*)LST_Next(pSection);
		}
	}
}

/*------------------------------------------------------------------------*/
/**# MODULE:READINI_ResetSection                                          */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * ResetSection
 *
 * SYNOPSIS
 *		void ResetSection(SectionTracker *pst)
 *
 * PURPOSE
 *		Resets <st> so GetNextINILine & FindNextINILine starts with
 *		the first config line.
 *
 * INPUT
 *		pst:				pointer to SectionTracker to reset
 *
 * AFFECTS
 *
 *
 * RETURN VALUE
 *		None.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void ResetSection(SectionTracker *pst)
{
	pst->clCurrentLine = (ConfigLine *) pst->ltSection;
	return;
}

/*------------------------------------------------------------------------*/
/**# MODULE:READINI_GetNextINILine                                        */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * GetNextINILine
 *
 * SYNOPSIS
 *		ConfigLine *GetNextINILine(SectionTracker *pst)
 *
 * PURPOSE
 *		Get LST_Next config line from a section. Update variables to
 *		reflect current line being processed.
 *
 * INPUT
 *		pst:				pointer to SectionTracker to get lines from
 *
 * AFFECTS
 *		pst->clCurrentLine is updated to point to LST_Next line.
 *
 * RETURN VALUE
 *		Updates and returns pst->CurrentLine, which is a
 *		pointer to config line, or NULL if no more lines.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/

ConfigLine *GetNextINILine(SectionTracker *pst)
{
	if (pst->clCurrentLine == (ConfigLine *) pst->ltSection)
	{
		pst->clCurrentLine = (ConfigLine*)LST_Head(pst->ltSection);
	}
	else
	{
		pst->clCurrentLine = (ConfigLine*)LST_Next(pst->clCurrentLine);
	}
	if (!LST_IsEOList(pst->clCurrentLine))
	{
		return pst->clCurrentLine;
	}
	else
	{
		return NULL;
	}
}

/*------------------------------------------------------------------------*/
/**# MODULE:READINI_CountINILines                                         */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CountINILines
 *
 * SYNOPSIS
 *		int CountINILines(SectionTracker *pst)
 *
 * PURPOSE
 *		Count number of config lines remaining in a section
 *		starting with current <pst>.
 *
 * INPUT
 *		pst:				pointer to SectionTracker to get lines from
 *
 * AFFECTS
 *
 * RETURN VALUE
 *		Returns number of config lines remaining in current section.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/

int CountINILines(SectionTracker *pst)
{
	ConfigLine *pLine;
	int wLines = 0;

	pLine = pst->clCurrentLine;			/* Save current line. */

	while(GetNextINILine(pst) != NULL)
		wLines++;

	pst->clCurrentLine = pLine;			/* Restore current line. */

	return wLines;
}

/*------------------------------------------------------------------------*/
/**# MODULE:READINI_GetNextString                                         */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * GetNextString
 *
 * SYNOPSIS
 *		BOOL GetNextString (SectionTracker *pst, char *sz, char *dst, size_t maxlen)
 *
 * PURPOSE
 *		Find the LST_Next line from a section that STARTS WITH
 *		(matches) the string <sz> and copy at most <maxlen - 1>
 *		bytes from the line to the string pointed to by <dst>
 *		appending a '\0' to the end.
 *
 * INPUT
 *		pst:				pointer to SectionTracker to get lines from
 *		sz:					name of configuration to match
 *		dst:				pointer to string to fill in
 *		maxlen:				max number of bytes to affect in dst.
 *
 * EFFECTS
 *		pst->clCurrentLine is updated to point to line that starts with <sz>.
 *
 * RETURN VALUE
 *		returns TRUE of dst was affected.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL GetNextString (SectionTracker *pst, const char *sz, char *dst, size_t maxlen)
{
	char	*s;

	s = FindNextINILine (pst, sz);
	if (s)
	{
		strncpy (dst, s, maxlen);
		dst[maxlen - 1] = '\0';
		return TRUE;
	}
	return FALSE;

} /* GetNextString */

/*------------------------------------------------------------------------*/
/**# MODULE:READINI_GetNextINT16                                          */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * GetNextINT16
 *
 * SYNOPSIS
 *		BOOL GetNextINT16 (SectionTracker *pst, char *sz, INT16 *dst)
 *
 * PURPOSE
 *		Find the LST_Next line from a section that STARTS WITH
 *		(matches) the string <sz> and sets the INT16 pointed to by
 *		<dst> to the value represented there.
 *
 *		If value starts with "0x" or "$" or ends with "h" the value
 *		will be converted as hex.
 *
 * INPUT
 *		pst:				pointer to SectionTracker to get lines from
 *		sz:					name of configuration to match
 *		dst:				pointer to INT16 to fill in
 *
 * EFFECTS
 *		pst->clCurrentLine is updated to point to line that starts with <sz>.
 *
 * RETURN VALUE
 *		returns TRUE of dst was affected.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL GetNextINT16 (SectionTracker *pst, const char *sz, INT16 *dst)
{
	char	*s;

	s = FindNextINILine (pst, sz);
	if (s)
	{
		
		*dst = EL_atos (s);
		return TRUE;
	}
	return FALSE;

} /* GetNextINT16 */

/*------------------------------------------------------------------------*/
/**# MODULE:READINI_GetNextUINT16                                         */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * GetNextUINT16
 *
 * SYNOPSIS
 *		BOOL GetNextUINT16 (SectionTracker *pst, char *sz, UINT16 *dst)
 *
 * PURPOSE
 *		Find the LST_Next line from a section that STARTS WITH
 *		(matches) the string <sz> and sets the UINT16 pointed to by
 *		<dst> to the value represented there.
 *
 *		If value starts with "0x" or "$" or ends with "h" the value
 *		will be converted as hex.
 *
 * INPUT
 *		pst:				pointer to SectionTracker to get lines from
 *		sz:					name of configuration to match
 *		dst:				pointer to UINT16 to fill in
 *
 * EFFECTS
 *		pst->clCurrentLine is updated to point to line that starts with <sz>.
 *
 * RETURN VALUE
 *		returns TRUE of dst was affected.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL GetNextUINT16 (SectionTracker *pst, const char *sz, UINT16 *dst)
{
	char	*s;

	s = FindNextINILine (pst, sz);
	if (s)
	{
		
		*dst = (UINT16)EL_atol (s);
		return TRUE;
	}
	return FALSE;

} /* GetNextUINT16 */

/*------------------------------------------------------------------------*/
/**# MODULE:READINI_GetNextINT32                                           */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * GetNextINT32
 *
 * SYNOPSIS
 *		BOOL GetNextINT32 (SectionTracker *pst, char *sz, INT32 *dst)
 *
 * PURPOSE
 *		Find the LST_Next line from a section that STARTS WITH
 *		(matches) the string <sz> and sets the INT32 pointed to by
 *		<dst> to the value represented there.
 *
 *		If value starts with "0x" or "$" or ends with "h" the value
 *		will be converted as hex.
 *
 * INPUT
 *		pst:				pointer to SectionTracker to get lines from
 *		sz:					name of configuration to match
 *		dst:				pointer to INT32 to fill in
 *
 * EFFECTS
 *		pst->clCurrentLine is updated to point to line that starts with <sz>.
 *
 * RETURN VALUE
 *		returns TRUE of dst was affected.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL GetNextINT32 (SectionTracker *pst, const char *sz, INT32 *dst)
{
	char	*s;

	s = FindNextINILine (pst, sz);
	if (s)
	{
		
		*dst = EL_atol (s);
		return TRUE;
	}
	return FALSE;

} /* GetNextINT32 */

/*------------------------------------------------------------------------*/
/**# MODULE:READINI_GetNextUINT32                                          */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * GetNextUINT32
 *
 * SYNOPSIS
 *		BOOL GetNextUINT32 (SectionTracker *pst, char *sz, UINT32 *dst)
 *
 * PURPOSE
 *		Find the LST_Next line from a section that STARTS WITH
 *		(matches) the string <sz> and sets the UINT32 pointed to by
 *		<dst> to the value represented there.
 *
 *		If value starts with "0x" or "$" or ends with "h" the value
 *		will be converted as hex.
 *
 * INPUT
 *		pst:				pointer to SectionTracker to get lines from
 *		sz:					name of configuration to match
 *		dst:				pointer to UINT32 to fill in
 *
 * EFFECTS
 *		pst->clCurrentLine is updated to point to line that starts with <sz>.
 *
 * RETURN VALUE
 *		returns TRUE of dst was affected.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL GetNextUINT32 (SectionTracker *pst, const char *sz, UINT32 *dst)
{
	char	*s;

	s = FindNextINILine (pst, sz);
	if (s)
	{
		
		*dst = EL_atol (s);
		return TRUE;
	}
	return FALSE;

} /* GetNextUINT32 */

/*------------------------------------------------------------------------*/
/**# MODULE:READINI_GetNextBool                                           */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * GetNextBool
 *
 * SYNOPSIS
 *		BOOL GetNextUINT32 (SectionTracker *pst, char *sz, BOOL *dst)
 *
 * PURPOSE
 *		Find the LST_Next line from a section that STARTS WITH
 *		(matches) the string <sz> and sets the BOOL pointed to by
 *		<dst> to the value represented there.
 *
 *		Values for 0 are	: 0
 *							: FALSE
 *							: No
 *							: N
 *							: F
 *							: Off
 *
 *		Values for 1 are	: 1 - N
 *							: TRUE
 *							: Yes
 *							: T
 *							: Y
 *							: On
 *	
 *		Note: Checking for TRUE/FALSE/YES/NO is NOT case sensitive
 *
 * INPUT
 *		pst:				pointer to SectionTracker to get lines from
 *		sz:					name of configuration to match
 *		dst:				pointer to BOOL to fill in
 *
 * EFFECTS
 *		pst->clCurrentLine is updated to point to line that starts with <sz>.
 *
 * RETURN VALUE
 *		returns TRUE of dst was affected.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
typedef struct {
	char	*s;
	BOOL	 val;
} BoolEntry;

static BoolEntry BoolTable[] = {
{	"0",		0,	},
{	"FALSE",	0,	},
{	"No",		0,	},
{	"N",		0,	},
{	"F",		0,	},
{	"Off",		0,	},
{	"1",		1,	},
{	"TRUE",		1,	},
{	"Yes",		1,	},
{	"T",		1,	},
{	"Y",		1,	},
{	"On",		1,	},
{	NULL,		0,  },
};

BOOL GetNextBool (SectionTracker *pst, const char *sz, BOOL *dst)
{
	char	*s;

	s = FindNextINILine (pst, sz);
	if (s)
	{
		if (atoi (s))
		{
			*dst = TRUE;
			return TRUE;
		}

		{
			BoolEntry	*be;

			be = BoolTable;
			while (be->s)
			{
				if (!stricmp (be->s, s))
				{
					*dst = be->val;
					return TRUE;
				}
				be++;
			}
		}
	}
	return FALSE;

} /* GetNextBool */

/*------------------------------------------------------------------------*/
/**# MODULE:READINI_FindFirstINILine                                      */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * FindFirstINILine
 *
 * SYNOPSIS
 *		char *FindFirstINILine (SectionTracker *pst, char *sz)
 *
 * PURPOSE
 *		Find the first line from a section that STARTS WITH
 *		(matches) the string <sz>.  Update variables to
 *		reflect current line being processed.
 *
 * INPUT
 *		pst:				pointer to SectionTracker to get lines from
 *		sz:					name of configuration to match
 *
 * EFFECTS
 *		pst->clCurrentLine is updated to point to line that starts with <sz>.
 *
 * RETURN VALUE
 *		Returns pointer to 1st character in matching string
 *		following <sz>, or NULL if no lines matches.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
char *FindFirstINILine (SectionTracker *pst, const char *sz)
{
	ResetSection (pst);
	return FindNextINILine (pst, sz);

} /* FindFirstINILine */

/*------------------------------------------------------------------------*/
/**# MODULE:READINI_GetFirstString                                        */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * GetFirstString
 *
 * SYNOPSIS
 *		BOOL GetFirstString (SectionTracker *pst, char *sz, char *dst, size_t maxlen)
 *
 * PURPOSE
 *		Find the first line from a section that STARTS WITH
 *		(matches) the string <sz> and copy at most <maxlen - 1>
 *		bytes from the line to the string pointed to by <dst>
 *		appending a '\0' to the end.
 *
 * INPUT
 *		pst:				pointer to SectionTracker to get lines from
 *		sz:					name of configuration to match
 *		dst:				pointer to string to fill in
 *		maxlen:				max number of bytes to affect in dst.
 *
 * EFFECTS
 *		pst->clCurrentLine is updated to point to line that starts with <sz>.
 *
 * RETURN VALUE
 *		returns TRUE of dst was affected.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL GetFirstString (SectionTracker *pst, const char *sz, char *dst, size_t maxlen)
{
	ResetSection (pst);
	return GetNextString (pst, sz, dst, maxlen);

} /* GetFirstString */

/*------------------------------------------------------------------------*/
/**# MODULE:READINI_GetFirstINT16                                         */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * GetFirstINT16
 *
 * SYNOPSIS
 *		BOOL GetFirstINT16 (SectionTracker *pst, char *sz, INT16 *dst)
 *
 * PURPOSE
 *		Find the first line from a section that STARTS WITH
 *		(matches) the string <sz> and sets the INT16 pointed to by
 *		<dst> to the value represented there.
 *
 *		If value starts with "0x" or "$" or ends with "h" the value
 *		will be converted as hex.
 *
 * INPUT
 *		pst:				pointer to SectionTracker to get lines from
 *		sz:					name of configuration to match
 *		dst:				pointer to INT16 to fill in
 *
 * EFFECTS
 *		pst->clCurrentLine is updated to point to line that starts with <sz>.
 *
 * RETURN VALUE
 *		returns TRUE of dst was affected.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL GetFirstINT16 (SectionTracker *pst, const char *sz, INT16 *dst)
{
	ResetSection (pst);
	return GetNextINT16 (pst, sz, dst);

} /* GetFirstINT16 */

/*------------------------------------------------------------------------*/
/**# MODULE:READINI_GetFirstUINT16                                        */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * GetFirstUINT16
 *
 * SYNOPSIS
 *		BOOL GetFirstUINT16 (SectionTracker *pst, char *sz, UINT16 *dst)
 *
 * PURPOSE
 *		Find the first line from a section that STARTS WITH
 *		(matches) the string <sz> and sets the UINT16 pointed to by
 *		<dst> to the value represented there.
 *
 *		If value starts with "0x" or "$" or ends with "h" the value
 *		will be converted as hex.
 *
 * INPUT
 *		pst:				pointer to SectionTracker to get lines from
 *		sz:					name of configuration to match
 *		dst:				pointer to UINT16 to fill in
 *
 * EFFECTS
 *		pst->clCurrentLine is updated to point to line that starts with <sz>.
 *
 * RETURN VALUE
 *		returns TRUE of dst was affected.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL GetFirstUINT16 (SectionTracker *pst, const char *sz, UINT16 *dst)
{
	ResetSection (pst);
	return GetNextUINT16 (pst, sz, dst);

} /* GetFirstUINT16 */

/*------------------------------------------------------------------------*/
/**# MODULE:READINI_GetFirstINT32                                          */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * GetFirstINT32
 *
 * SYNOPSIS
 *		BOOL GetFirstINT32 (SectionTracker *pst, char *sz, INT32 *dst)
 *
 * PURPOSE
 *		Find the first line from a section that STARTS WITH
 *		(matches) the string <sz> and sets the INT32 pointed to by
 *		<dst> to the value represented there.
 *
 *		If value starts with "0x" or "$" or ends with "h" the value
 *		will be converted as hex.
 *
 * INPUT
 *		pst:				pointer to SectionTracker to get lines from
 *		sz:					name of configuration to match
 *		dst:				pointer to INT32 to fill in
 *
 * EFFECTS
 *		pst->clCurrentLine is updated to point to line that starts with <sz>.
 *
 * RETURN VALUE
 *		returns TRUE of dst was affected.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL GetFirstINT32 (SectionTracker *pst, const char *sz, INT32 *dst)
{
	ResetSection (pst);
	return GetNextINT32 (pst, sz, dst);

} /* GetFirstINT32 */

/*------------------------------------------------------------------------*/
/**# MODULE:READINI_GetFirstUINT32                                         */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * GetFirstUINT32
 *
 * SYNOPSIS
 *		BOOL GetFirstUINT32 (SectionTracker *pst, char *sz, UINT32 *dst)
 *
 * PURPOSE
 *		Find the first line from a section that STARTS WITH
 *		(matches) the string <sz> and sets the UINT32 pointed to by
 *		<dst> to the value represented there.
 *
 *		If value starts with "0x" or "$" or ends with "h" the value
 *		will be converted as hex.
 *
 * INPUT
 *		pst:				pointer to SectionTracker to get lines from
 *		sz:					name of configuration to match
 *		dst:				pointer to UINT32 to fill in
 *
 * EFFECTS
 *		pst->clCurrentLine is updated to point to line that starts with <sz>.
 *
 * RETURN VALUE
 *		returns TRUE of dst was affected.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL GetFirstUINT32 (SectionTracker *pst, const char *sz, UINT32 *dst)
{
	ResetSection (pst);
	return GetNextUINT32 (pst, sz, dst);

} /* GetFirstUINT32 */

/*------------------------------------------------------------------------*/
/**# MODULE:READINI_GetFirstBool                                          */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * GetFirstBool
 *
 * SYNOPSIS
 *		BOOL GetFirstUINT32 (SectionTracker *pst, char *sz, BOOL *dst)
 *
 * PURPOSE
 *		Find the first line from a section that STARTS WITH
 *		(matches) the string <sz> and sets the BOOL pointed to by
 *		<dst> to the value represented there.
 *
 *		Values for 0 are	: 0
 *							: FALSE
 *							: No
 *							: N
 *							: F
 *
 *		Values for 1 are	: 1 - N
 *							: TRUE
 *							: Yes
 *							: T
 *							: Y
 *	
 *		Note: Checking for TRUE/FALSE/YES/NO is NOT case sensitive
 *
 * INPUT
 *		pst:				pointer to SectionTracker to get lines from
 *		sz:					name of configuration to match
 *		dst:				pointer to BOOL to fill in
 *
 * EFFECTS
 *		pst->clCurrentLine is updated to point to line that starts with <sz>.
 *
 * RETURN VALUE
 *		returns TRUE of dst was affected.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
BOOL GetFirstBool (SectionTracker *pst, const char *sz, BOOL *dst)
{
	ResetSection (pst);
	return GetNextBool (pst, sz, dst);

} /* GetFirstBool */
