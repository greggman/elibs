/*
 * DEFINER.C
 *
 *  COPYRIGHT : 1992 Echidna
 * PROGRAMMER : Juan M. Alvarado
 *    VERSION : 00.000
 *    CREATED : 10/16/92
 *   MODIFIED : 12/13/93
 *       TABS : 04 08
 *
 * DESCRIPTION
 *		I want to consolidate constants that are shared by assembly and
 *		c files into a single file so I make sure the assembly and c 
 *		routines are using the same constant values.  So I wrote this 
 *		program that will take the stuff from one file and spit out the
 *		appropriate c and assembly files containing the same information.
 *		But, since I am smart ;-) I made the format of this source file 
 *		very configurable.  See DEFINER.DOC for documentation.
 *
 * HISTORY
 *		11/30/96 : Fixed for EL
 *
 *
*/

#include "platform.h"
#include "switches.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <echidna/eio.h>
#include <echidna/utils.h>
#include <echidna/eerrors.h>
#include <echidna/listapi.h>
#include <echidna/argparse.h>
#include <echidna/strings.h>
#include <echidna/version.h>

/**************************** C O N S T A N T S ***************************/

#define MAX_LINE_SIZE	256
#define MAX_ARGS		80

#define SAME_CASE	0
#define LOWER_CASE	1
#define UPPER_CASE	2

/******************************** T Y P E S *******************************/

typedef struct {
	/* The output strings are in string LST_NodeName(Node) */
	LST_NODE	Node;
	short		IsBody;	/* True if this is the place to emit body 
						 * otherwise just:
						 *   puts(DefPrintS(str, LST_NodeName(Node))) */
} OutNodeType;

typedef struct {
	/* The rule name is in string LST_NodeName(Node) */
	LST_NODE	Node;
	LST_LIST	ArgList;
} RuleNodeType;

typedef struct {
	/* The format tags are in string LST_NodeName(Node) (e.g. "c .h")*/
	LST_NODE	Node;
	LST_LIST	OutList;
	LST_LIST	RuleList;

} FormatType;

typedef struct {
	/* name of output file is in LST_NodeName(Node) */
	LST_NODE	Node;
	FormatType	*Format;
} OutFileType;

LST_LIST	FormatsList;
LST_LIST	DefList;	/* definitions from source file */

typedef enum {CFG_SECTION, CFG_LITERAL, CFG_COMMAND, CFG_RULE} CFGLineType;

typedef enum {SEC_BAD, SEC_OUTPUT, SEC_END} SectionType;

/****************************** G L O B A L S *****************************/

short	DebugSwitch = FALSE;
char	CFGFile[EIO_MAXPATH] = "DEFINER.CFG";
short		LineNo;
char	CrntDirPath[EIO_MAXPATH] = {0};
char	SrcFile[EIO_MAXPATH];
char	*DstFile = "";

typedef enum {KW_BAD, KW_OUTPUT, KW_BODY, KW_END} KeyWordType;
static char *KeyWords[] = {
	"",	
	"OUTPUT",
	"BODY",
	"END",
	NULL,
};

typedef enum {
	CA_NO_MATCH,
	CA_ARG,
	CA_DAY,
	CA_DBASE,
	CA_DEXT,
	CA_DNAME,
	CA_DPATH,
	CA_ENV,
	CA_HOUR,
	CA_HR,
	CA_MER,
	CA_MIN,
	CA_MONTH,
	CA_NDAY,
	CA_NMONTH,
	CA_SBASE,
	CA_SEC,
	CA_SEXT,
	CA_SNAME,
	CA_SPATH,
	CA_VER,
	CA_YEAR,
	CA_YR,
} CFGArgType;

#define NUM_CFG_ARG_TYPES	23	
/* Must be sorted alphabetically for binary search */
/* No name can be a starting substring of another or it will only ever
 * match the shorter name when searching */
static char *CFGArgStrs[NUM_CFG_ARG_TYPES] = {
	"arg",
	"day",
	"dbase",
	"dext",
	"dname",
	"dpath",
	"env",
	"hour",
	"hr",
	"mer",
	"min",
	"month",
	"nday",
	"nmonth",
	"sbase",
	"sec",
	"sext" ,
	"sname",
	"spath",
	"ver",
	"year",
	"yr",
};

static char	**SArgs;	/* Arguments list to definer source file */

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/

/*********************************************************************
 *
 * KeyWordFind
 *
 * SYNOPSIS
 *		short	KeyWordFind (char **table, short numentries, char *str)
 *
 * PURPOSE
 *		To find the first keyword from table in str.  
 *		Warning: matches shortest string.
 *
 * INPUT
 *
 *
 * AFFECTS
 *
 *
 * RETURN VALUE
 *		Success: Returns index of keyword match + 1 (1..numentries).
 *		Failure: Returns 0.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
short	KeyWordFind (char **table, short numentries, char *str)
{
/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "KeyWordFind";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
{
	short matchindex = 0;	
	short a, b, c;
	
	a = 0; c = numentries-1;
	while (!matchindex) {
		char *s, *stry;
		
		b= a + (c - a)/2;

		/* see how far the strings match */
		for (s=str, stry=table[b]; (*s && *stry && *s == *stry); ++s, ++stry);
		
		if (*stry == '\0') { /* Match */
			matchindex = b + 1;
			break;
		} else if (*s == '\0') { /* No match */
			break;
		} else { /* No match yet */
			if (a >= c) { /* No more to search; no match */
				break;
			}
			if (*s > *stry) {
				a = b + 1;				
			} else {
				c = b - 1;
			}
		}
	}

	return matchindex;
}
} /* KeyWordFind */

/*********************************************************************
 *
 * GetLine
 *
 * SYNOPSIS
 *		char *GetLine (char *line, size_t size, FILE *fp)
 *
 * PURPOSE
 *		Read a line from a text file into specifed buffer.
 *		* Remove ';' style comments.
 *		* Strip Leading and Trailing WhiteSpace.
 *		* Skip "Blank" Lines.
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
char *GetLine (char *line, size_t size, FILE *fp)
{
	char	*s;
/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetLine";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
	do {
		s = fgets (line, size, fp);
		if (s) {
			LineNo++;
			StripComment (s);
			s = TrimWhiteSpace (s);
			if (strlen (s)) {
				return s;
			}
		}
	} while (s);
	return NULL;

} /* GetLine */

/*********************************************************************
 *
 * GetKeyWord
 *
 * SYNOPSIS
 *		KeyWordType GetKeyWord (char *line)
 *
 * PURPOSE
 *		To see if the string starts with a keyword.
 *
 * INPUT
 *
 *
 * AFFECTS
 *
 *
 * RETURN VALUE
 *		Success: keyword value.
 *		Failure: KW_BAD
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
KeyWordType GetKeyWord (char *line)
{
/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetKeyWord";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
{	
	KeyWordType i = KW_BAD + 1;
	KeyWordType kw = KW_BAD;

	while (KeyWords[i]) {
		char *kwstr = KeyWords[i];
		short matchlen = 0;
		short n = 0;

		for (n = 0; toupper (line[n]) == kwstr[n]; ++matchlen, n++);

		if (matchlen = strlen (KeyWords[i]) && !isalpha (line[matchlen]))
		{
			kw = i;
			break;
		}

		++i;
	}
	return kw;
}
} /* GetKeyWord */

/*********************************************************************
 *
 * GetSectionType
 *
 * SYNOPSIS
 *		SectionType	GetSectionType (
 *			char	*line,
 *			char	**psz
 *		) 
 *
 * PURPOSE
 *		To check section header for syntax errors and return the section
 *		type and any data after the equals sign.
 *
 * INPUT
 *		line:	Section line. First char of string must be '['.
 *		psz:  	String ptr that will be set to the data after the '='
 *					 if there is any, or NULL if there is none.
 *	
 * AFFECTS
 *
 *
 * RETURN VALUE
 *		Success: type of section.
 *		Failure: SEC_BAD.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
SectionType	GetSectionType (
	char	*line,
	char	**psz
) 
{
/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetSectionType";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
{
	short len;
	SectionType sectype = SEC_BAD;

	*psz = NULL;
	++line; 	/* skip over open bracket */
	len = strlen (line);
	if (line[len-1] == ']') {
		char *c;

		line[len-1] = '\0';

		switch (GetKeyWord (line)) {
		case KW_OUTPUT:
			if (c = strpbrk (line, "=")) {
				*c = '\0';
				line = c + 1;
				if (strlen(line) > 0) {
					sectype = SEC_OUTPUT;
					*psz = line;		
				} else {
					ErrMess ("Syntax: CFG file '%s': Line %d: Output Section header missing type tags.\n",
						CFGFile, LineNo);
				}
			} else {
				ErrMess ("Syntax: CFG file '%s': Line %d: Output Section header missing '='.\n",
					CFGFile, LineNo);
			}
			break;
		case KW_END:
			sectype = SEC_END;
			break;
		}
	} else {
		ErrMess ("Syntax: CFG file '%s': Line %d: Section header missing ']'.\n",
			CFGFile, LineNo);
	}

	return sectype;
}
} /* GetSectionType */

/*********************************************************************
 *
 * ReadConfig
 *
 * SYNOPSIS
 *		LST_LIST	*ReadConfig (void)
 *
 * PURPOSE
 *		To read the configuration file and collect all the output formats.
 *
 * INPUT
 *		filename: Configuration file name.
 *
 * AFFECTS
 *		
 *
 * RETURN VALUE
 *		Success: Pointer to formats list.
 *		Failure: NULL.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
LST_LIST	*ReadConfig (void)
{
/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "ReadConfig";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
{
	LST_LIST	*formatslist;
	FormatType	*crnt_format_node = NULL;
	char		 pathbuf[EIO_MAXPATH];

	if (EIO_FindFile (CFGFile, CFGFile, pathbuf)) {
		strcpy (CFGFile, pathbuf);
		if (formatslist = LST_CreateList (CFGFile)) {
			FILE		*fp;
			if (fp = fopen (CFGFile, "r")) {
				static char	 Line[MAX_LINE_SIZE];
				char		*line;
				char		*args[MAX_ARGS + 1];

				LineNo = 0;
				while ((line = GetLine (Line, MAX_LINE_SIZE, fp)) != NULL) {
					SectionType sectype;
					char	*datastr;
					
					switch (*line) {
					case '[': 
						if (sectype = GetSectionType (line, &datastr))
						{
							switch (sectype) {
							case SEC_OUTPUT:
								strupr (datastr);
								if (crnt_format_node = (FormatType*)LST_CreateNode (sizeof (*crnt_format_node),
									datastr))
								{
									LST_InitList (&(crnt_format_node->OutList));
									LST_InitList (&(crnt_format_node->RuleList));
									LST_AddTail (formatslist, crnt_format_node);
								} else {
									OutOfMemErr ("CFG file '%s': Line %d: creating section node.\n",
										CFGFile, LineNo);
								}
								break;
							case SEC_END:
								if (crnt_format_node) {
									crnt_format_node = NULL;
								} else {
									WarnMess ("Syntax: CFG file '%s': Line %d: superfluous END section.\n",
										CFGFile, LineNo);
								}
								break;
							}
						}
						break;
					case '<': 
						if (crnt_format_node) {
							KeyWordType kw = GetKeyWord (line + 1);
							if (kw == KW_BODY) {
								OutNodeType *outnode;
								if (outnode = (OutNodeType*)LST_CreateNode (sizeof (*outnode),
									NULL))
								{
									outnode->IsBody = TRUE;
									LST_AddTail (&(crnt_format_node->OutList), outnode);
								} else {
									OutOfMemErr ("CFG file '%s': creating command outnode.\n",
										CFGFile, LineNo);
								}
							} else {
								ErrMess ("Syntax: CFG file '%s': Line %d: unknown <command>.\n",
									CFGFile, LineNo);
							}
						} else {
							WarnMess ("Syntax: CFG file '%s': Line %d: <commands> may only appear in output sections.\n",
								CFGFile, LineNo);
						}
						break;
					case '"': 
						if (crnt_format_node) {
							OutNodeType *outnode;
							short argcount = argify (line, 1, args);
							if (argcount == 0){
								args[0] = "";
							}
							if (outnode = (OutNodeType*)LST_CreateNode (sizeof (*outnode),
								args[0]))
							{
								LST_AddTail (&crnt_format_node->OutList, outnode);
							} else {
								OutOfMemErr ("CFG file '%s': creating literal outnode.\n",
									CFGFile, LineNo);
							}
						} else {
							WarnMess ("Syntax: CFG file '%s': Line %d: literals can only appear in output sections.\n",
								CFGFile, LineNo);
						}
						break;
					default: /* Is Rule */
						if (crnt_format_node) {
							RuleNodeType *rulenode;
							short argcount = argify (line, MAX_ARGS, args);
							short i;

							if (rulenode = (RuleNodeType*)LST_CreateNode (sizeof (*rulenode),
								args[0]))
							{
								LST_InitList (&rulenode->ArgList);
								for (i = 1; i < argcount; i++) {
									LST_NODE *argnode;
									if (argnode = LST_CreateNode (sizeof (*argnode),
										args[i]))
									{
										LST_AddTail (&rulenode->ArgList, argnode);
									} else {
										OutOfMemErr ("CFG file '%s': creating rule arg node.\n",
											CFGFile, LineNo);
									}
								}
								LST_AddTail (&crnt_format_node->RuleList, rulenode);
							} else {
								OutOfMemErr ("CFG file '%s': creating rule node.\n",
									CFGFile, LineNo);
							}
						} else {
							WarnMess ("Syntax: CFG file '%s': Line %d: rules may only appear in output sections.\n",
								CFGFile, LineNo);
						}
						break;
					}
				}
				fclose (fp);
			} else {
				ErrMess ("Could not open config file '%s'.\n", CFGFile);
			}	
		} else {
			ErrMess ("Out of memory allocating formats list for config file '%s'.\n", 
				CFGFile);
		}
	} else {
		ErrMess ("Can't find config file '%s'.\n", CFGFile);
	}

	return formatslist;
}
} /* ReadConfig */

/************************************************************************/
#if DEBUGGING
LST_LIST *PrintFormats (LST_LIST *flist)
{
	if (DebugSwitch && flist) {
		FormatType *ft;
		ft = (FormatType *)LST_Head(flist);
		while (!LST_IsEOList (ft)) {
			OutNodeType *nt;
			RuleNodeType *rt;

			EL_printf ("[output=%s]\n", LST_NodeName (ft));
			nt = (OutNodeType *)LST_Head(&ft->OutList);
			while (!LST_IsEOList (nt)) {
				if (nt->IsBody) {
					EL_printf ("\t<body>\n");
				} else {
					EL_printf ("\t\"%s\"\n", LST_NodeName (nt));
				}
				nt = (OutNodeType*)LST_Next (nt);
			}
			rt = (RuleNodeType *)LST_Head (&ft->RuleList);
			while (!LST_IsEOList (rt)) {
				LST_NODE *n;
				EL_printf ("\t%s", LST_NodeName (rt));
				n = (LST_NODE *)LST_Head (&rt->ArgList);
				while (!LST_IsEOList (n)) {
					EL_printf (" \"%s\"", LST_NodeName(n));
					n = LST_Next (n);
				}
				puts ("");
				rt = (RuleNodeType*)LST_Next (rt);
			}
			ft = (FormatType*)LST_Next (ft);
		}	
	}
	return flist;
} /* PrintFormats */
#else
#define PrintFormats(flist)	(flist)
#endif

/*********************************************************************
 *
 * GetFormat
 *
 * SYNOPSIS
 *		FormatType *GetFormat (LST_LIST *formatlist, char *tag)
 *
 * PURPOSE
 *		To get the format node with the matching tag.
 *
 * INPUT
 *
 *
 * AFFECTS
 *
 *
 * RETURN VALUE
 *		Success: pointer to FormatType struct.
 *		Failure: NULL.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
FormatType *GetFormat (LST_LIST *formatlist, char *tag)
{
/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetFormat";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
{
	FormatType *ft, *format;
	short len = strlen (tag);

	format = NULL;

	strupr (tag);
	ft = (FormatType *)LST_Head (formatlist);
	while (!LST_IsEOList (ft)) {
		char *fstr;

		if (fstr = strstr (LST_NodeName (ft),tag)) {
			if (isspace (fstr[len]) || fstr[len] == '\0') {
				format = ft;
				break;
			}
		}
		ft = (FormatType*)LST_Next (ft);
	}
	return format;
}
} /* GetFormat */

/*********************************************************************
 *
 * GetCFGArgSubst
 *
 * SYNOPSIS
 *		char	*GetCFGArgSubst (
 *			CFGArgType cfgat,
 *			short	alphacase,
 *			short	leftjustify,
 *			short	fhaswidth,
 *			short	fhasprec,
 *			short	width,
 *			short	prec,
 *			char	**srcstr
 *		)
 *
 * PURPOSE
 *		To create the string asked for and return pointer to static copy.
 *
 * INPUT
 *		cfgat: Must be valid.
 *		
 * AFFECTS
 *
 *
 * RETURN VALUE
 *		Pointer to static string.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
char	*GetCFGArgSubst (
	CFGArgType cfgat,
	short	alphacase,
	short	leftjustify,
	short	fhaswidth,
	short	fhasprec,
	short	width,
	short	prec,
	char	**srcstr
)
{
/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetCFGArgSubst";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
{
	static char outstr[MAX_LINE_SIZE];
	char tmpstr[MAX_LINE_SIZE];
	char frmtstr[MAX_LINE_SIZE];
	time_t	stime_t;
	struct tm	*ptm;
	char	*str = *srcstr;

	time (&stime_t);
	ptm = localtime (&stime_t);

	frmtstr[0] = tmpstr[0] = outstr[0] = '\0';

	switch (cfgat) {
	case CA_DAY:
		strftime (tmpstr, MAX_LINE_SIZE, "%d", ptm);
		break;
	case CA_DBASE:
		EIO_fnsplit (DstFile, NULL, tmpstr, NULL);
		break;
	case CA_DEXT:
		if (EIO_EXTENSION & EIO_fnsplit (DstFile, NULL, NULL, outstr)) {
			strcpy (tmpstr, outstr + 1);
		}
		break;
	case CA_DNAME:
		EIO_fnsplit (DstFile, NULL, tmpstr, NULL);
		EIO_fnsplit (DstFile, NULL, NULL, outstr);
		strcat (tmpstr, outstr);
		break;
	case CA_DPATH:
		{ 
			char pathstr[EIO_MAXPATH];
			char *d, *s;

			EIO_fnsplit (DstFile, pathstr, NULL, NULL);
			if (pathstr[0] == '\0') {
				strcpy (pathstr, CrntDirPath);
			}
			/* interpret backslashes */
			d = tmpstr; s = pathstr;
			while (*s) {
				if (*s == '\\') {
					*d++ = *s++;
					*d++ = '\\';
				} else {
					*d++ = *s++;
				}
			}
			*d = '\0';
		}
		break;
	case CA_ENV:
		{ char chOpen;
			if ( (chOpen = *str) == '<' || chOpen == '[') {
				char chClose = (chOpen == '<') ? '>' : ']';
				char *envstrname = ++str;
				while (*str && *str != chClose) ++str;
				if (*str == chClose) {
					char *envstr;
					*str = '\0';
					if (envstr = getenv (envstrname)) {
						if (chOpen == '[') { /* uninterpreted string */
							strcpy (tmpstr, envstr);
						} else  { /* interpret backslashes */
							char *d = tmpstr;
							while (*envstr) {
								if (*envstr == '\\') {
									*d++ = *envstr++;
									*d++ = '\\';
								} else {
									*d++ = *envstr++;
								}
							}
							*d = '\0';
						}
					}
					*str = chClose;
					*srcstr = str + 1;
				} else {
					ErrMess ("Source file '%s': Line %d: Missing %c. Unterminated environment variable name.\n",
						SrcFile, LineNo, chClose);
				}
			} else {
				ErrMess ("Source file '%s': Line %d: Missing '<' or '[' in @env arg.\n",
					SrcFile, LineNo);
			}
		}
		break;
	case CA_ARG:
		{ char chOpen;
			if ((chOpen = *str) == '<' || chOpen == '[') {
				char chClose = (chOpen == '<') ? '>' : ']';
				short argnum = 0;
				++str;
				if (isdigit (*str)) {
					do {
						argnum = (argnum * 10) + (*str - '0');
						++str;
					} while (isdigit (*str));
					if (*str == chClose) {
						char *argstr = NULL;
						if (SArgs) {
							short i;
							for (i=0; (SArgs[i] && i < argnum); i++);
							if (i == argnum) {
								argstr = SArgs[argnum-1];
							}
						}
						if (argstr) {
							if (chOpen == '[') { /* uninterpreted string */
								strcpy (tmpstr, argstr);
							} else  { /* interpret backslashes */
								char *d = tmpstr;
								while (*argstr) {
									if (*argstr == '\\') {
										*d++ = *argstr++;
										*d++ = '\\';
									} else {
										*d++ = *argstr++;
									}
								}
								*d = '\0';
							}
						}
						*srcstr = str + 1;
					} else {
						ErrMess ("Source file '%s': Line %d: Missing %c. Unterminated '@arg<>' variable number.\n",
							SrcFile, LineNo, chClose);
					}
				} else {
						ErrMess ("Source file '%s': Line %d: Number required inside '@arg<>' angle brackets.\n",
							SrcFile, LineNo);
				}
			} else {
				ErrMess ("Source file '%s': Line %d: Missing '<' or '[' in '@arg<>'.\n",
					SrcFile, LineNo);
			}
		}
		break;
	case CA_HOUR:
		strftime (tmpstr, MAX_LINE_SIZE, "%H", ptm);
		break;
	case CA_HR:
		strftime (tmpstr, MAX_LINE_SIZE, "%I", ptm);
		break;
	case CA_MER:
		sprintf (tmpstr, "%s", (ptm->tm_hour < 12) ? "a.m." : "p.m.");
		break;
	case CA_MIN:
		strftime (tmpstr, MAX_LINE_SIZE, "%M", ptm);
		break;
	case CA_MONTH:
		strftime (tmpstr, MAX_LINE_SIZE, "%m", ptm);
		break;
	case CA_NDAY:
		strftime (tmpstr, MAX_LINE_SIZE, "%A", ptm);
		break;
	case CA_NMONTH:
		strftime (tmpstr, MAX_LINE_SIZE, "%B", ptm);
		break;
	case CA_SBASE:
		EIO_fnsplit (SrcFile, NULL, tmpstr, NULL);
		break;
	case CA_SEC:
		strftime (tmpstr, MAX_LINE_SIZE, "%S", ptm);
		break;
	case CA_SEXT:
		if (EIO_EXTENSION & EIO_fnsplit (SrcFile, NULL, NULL, outstr)) {
			strcpy (tmpstr, outstr + 1);
		}
		break;
	case CA_SNAME:
		EIO_fnsplit (SrcFile, NULL, tmpstr, NULL);
		EIO_fnsplit (SrcFile, NULL, NULL, outstr);
		strcat (tmpstr, outstr);
		break;
	case CA_SPATH:
		{ 
			char pathstr[EIO_MAXPATH];
			EIO_fnsplit (SrcFile, pathstr, NULL, NULL);
			if (pathstr[0] == '\0') {
				strcpy (pathstr, CrntDirPath);
			}
			{ /* interpret backslashes */
				char *d = tmpstr; char *s = pathstr;
				while (*s) {
					if (*s == '\\') {
						*d++ = *s++;
						*d++ = '\\';
					} else {
						*d++ = *s++;
					}
				}
				*d = '\0';
			}
		}

		break;
	case CA_VER:
		strcpy (tmpstr, Version);
		break;
	case CA_YEAR:
		strftime (tmpstr, MAX_LINE_SIZE, "%Y", ptm);
		break;
	case CA_YR:
		strftime (tmpstr, MAX_LINE_SIZE, "%y", ptm);
		break;
	}

	if (alphacase == UPPER_CASE) {
		strupr (tmpstr);
	} else if (alphacase == LOWER_CASE) {
		strlwr (tmpstr);
	}

	strcpy (frmtstr, "%");
	if (leftjustify) {
		strcat (frmtstr, "-");
	}
	if (fhaswidth) {
		sprintf (outstr, "%d", width);
		strcat (frmtstr, outstr);
	}
	if (fhasprec) {
		sprintf (outstr, ".%d", prec);
		strcat (frmtstr, outstr);
	}
	strcat (frmtstr, "s");

	sprintf (outstr, frmtstr, tmpstr);

	return outstr;
}
} /* GetCFGArgSubst */

/*********************************************************************
 *
 * SubstCFGArgs
 *
 * SYNOPSIS
 *		char	*SubstCFGArgs (char *str)
 *
 * PURPOSE
 *		To substitute info for the @ formating substrings.
 *
 * INPUT
 *
 *
 * AFFECTS
 *
 *
 * RETURN VALUE
 *		Pointer to static string.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
char	*SubstCFGArgs (char *str)
{
/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SubstCFGArgs";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
{
	static char outstr[MAX_LINE_SIZE];
	char *c = outstr;
	short count = 0;
	char	*s = str;

	while (*s && count < MAX_LINE_SIZE-1) {
		if ('@' == *s) {
			char	buf[MAX_LINE_SIZE];
			short	len;
			short	kw;
			short	alphacase = SAME_CASE;
			short	fhaswidth = FALSE;
			short	fhasprec = FALSE;
			short	width, prec;
			short	leftjustify = FALSE;

			++s;

			while (lchinstr (*s, "ul-") >= 0) {
				/* Get flags field */
				switch (*s) {
				case 'u':
					alphacase = UPPER_CASE; 
					++s; 
					break;
				case 'l': 
					alphacase = LOWER_CASE; 
					++s; 
					break;
				case '-':
					leftjustify = TRUE;
					++s;
					break;
				}
			}

			/* Get width field */
			if (isdigit (*s)) {
				fhaswidth = TRUE;
				width = 0;
				do {
					width = (width * 10) + (*s - '0');
					++s;
				} while (isdigit (*s));
			}

			/* Get prec field */
			if (*s == '.' && isdigit(s[1])) {
				++s;
				fhasprec = TRUE;
				prec = 0;
				do {
					prec = (prec * 10) + (*s - '0');
					++s;
				} while (isdigit (*s));
			}

			switch (kw = KeyWordFind (CFGArgStrs, NUM_CFG_ARG_TYPES, s)) {
			default:
				s += strlen (CFGArgStrs [kw-1]);
				strcpy (buf, GetCFGArgSubst (kw,
					alphacase, leftjustify, 
					fhaswidth, fhasprec, width, prec, &s));
				break;
			case CA_NO_MATCH:
				if (*s == '@') {
					buf[0] = '@'; buf[1] = '\0';
					++s;
				} else {
					buf[0] = '\0';
					ErrMess ("Source file '%s': Line %d: Unknown @ arg type '%s'.\n",
						SrcFile, LineNo, s -1);
				}
				break;
			}

			if ((len = strlen (buf)) < MAX_LINE_SIZE - count) {
				strcpy (c, buf);
				count += len;
				c += len;
			} else {
				count = MAX_LINE_SIZE;
				break;
			}
		} else {
			*c++ = *s++;
			++count;
		}
	}

	if (*s != '\0') {
		ErrMess ("Source file '%s': Line %d: Max line len (%d) exceeded.\n",
			SrcFile, LineNo, MAX_LINE_SIZE);
	}

	*c = '\0';

	return outstr;
}
} /* SubstCFGArgs */

/*********************************************************************
 *
 * SubstSrcArgs
 *
 * SYNOPSIS
 *		char	*SubstSrcArgs (char *rule, LST_LIST *args)
 *
 * PURPOSE
 *		To create a new string from the rule, substituting the args in
 *		the appropriate places.
 *
 * INPUT
 *		rule: rule string with %# formating (e.g. "#define %1 %2")
 *		args: a list of argument strings.
 *	
 *	
 * AFFECTS
 *
 *
 * RETURN VALUE
 *		Pointer to static string.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
char	*SubstSrcArgs (char *rule, LST_LIST *args)
{
/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "SubstSrcArgs";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
{
	static char outstr[MAX_LINE_SIZE];
	char *c = outstr;
	short count = 0;
	LST_NODE	*arg;
	char	*r = rule;
	short	ididarg = 0;
	
	while (*r && count < MAX_LINE_SIZE-1) {
		if ('%' == *r) {
			int		alphacase = SAME_CASE;
			
			++r;
			switch (tolower(*r))
			{
			case 'l':
				++r;
				alphacase = LOWER_CASE;
				break;
			case 'u':
				++r;
				alphacase = UPPER_CASE;
				break;
			}
			if (isdigit (*r)) {
				short index = 0;	
				do {
					index= (index * 10) + (*r - '0');
					++r;
				} while (isdigit (*r));
				
				if (index > 0) {
					ididarg = index;
					arg = (LST_NODE *)LST_Head (args);
					for (--index; (!LST_IsEOList (arg) && index); index--) {
						arg = LST_Next (arg);
					}
					if (!LST_IsEOList (arg)) {
						short len = strlen (LST_NodeName (arg));
						if (len < MAX_LINE_SIZE - count) {
							strcpy (c, LST_NodeName (arg));
							switch (alphacase)
							{
							case UPPER_CASE:
								strupr (c);
								break;
							case LOWER_CASE:
								strlwr (c);
								break;
							}

							count += len;
							c += len;
						}
					} else {
						ErrMess ("Output File '%s': Line %d: Too few args for rule '%s'.\n",
							DstFile, LineNo, rule);
					}
				} else {
					ErrMess ("Output File '%s': Line %d: %0 is illegal. Args start at 1.\n",
						DstFile, LineNo);
				}

			} else if (*r == '*') {
				short index = ididarg;
				++r;
				arg = (LST_NODE *)LST_Head (args);
				/* get current arg */
				for (index = ididarg; (!LST_IsEOList (arg) && index); index--) {
					arg = LST_Next (arg);
				}

				while (!LST_IsEOList (arg)) {
					short len = strlen (LST_NodeName (arg));
					++ididarg;
					if (len < MAX_LINE_SIZE - (count)) {
						strcpy (c, LST_NodeName (arg));
						switch (alphacase)
						{
						case UPPER_CASE:
							strupr (c);
							break;
						case LOWER_CASE:
							strlwr (c);
							break;
						}
						count += len;
						c += len;
					} else {
						count = MAX_LINE_SIZE;
						break;
					}
					arg = LST_Next (arg);
					if (!LST_IsEOList (arg) && len < MAX_LINE_SIZE) {
						*c++ = ' ';
						++count;
					}
				}
			} else {
				*c++ = *r++;
				++count;
			}
		} else {
			*c++ = *r++;
			++count;
		}
	}

	if (*r != '\0') {
		ErrMess ("Output file '%s': Line %d: Max line len (%d) exceeded.\n",
			DstFile, LineNo, MAX_LINE_SIZE);
	}

	*c = '\0';
	
	return outstr;
}
} /* SubstSrcArgs */


/*********************************************************************
 *
 * PrintBody
 *
 * SYNOPSIS
 *		void PrintBody (FILE *fp, FormatType format, LST_LIST defs)
 *
 * PURPOSE
 *		To print out the body (defs) using the rules in 'format'.
 *
 * INPUT
 *
 *
 * AFFECTS
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
void PrintBody (FILE *fp, FormatType *format, LST_LIST *defs)
{
/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "PrintBody";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
{
	RuleNodeType	*r, *d;
	LST_LIST		*rules = &format->RuleList;

	d = (RuleNodeType *)LST_Head (defs);
	while (!LST_IsEOList (d)) {
		if (r = (RuleNodeType*)LST_FindIName (rules, LST_NodeName (d))){
			char *s = LST_NodeName (LST_Head (&r->ArgList));
			++LineNo;
			if (!s) s = "";
			s = SubstSrcArgs (s, &d->ArgList);
			s = SubstCFGArgs (s);
			UnEscStr (s);
			fprintf (fp, (s));
			fprintf (fp, "\n");
		} else {
			WarnMess ("'%s' Rule not found for '%s' format.\n", LST_NodeName (d),
				LST_NodeName(format));
		}
		d = (RuleNodeType*)LST_Next (d);
	}
}
} /* PrintBody */

/*********************************************************************
 *
 * WriteOutputFile
 *
 * SYNOPSIS
 *		void	*WriteOutputFile (void *outfilenode)
 *
 * PURPOSE
 *		To write the output file, of course. 
 *
 * INPUT
 *
 *
 * AFFECTS
 *
 *
 * RETURN VALUE
 *		Returns Next node in list.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void	*WriteOutputFile (void *node)
{
/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "WriteOutputFile";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
{
 	FILE		*fp;
	OutFileType *of = node;

	LineNo = 0;
	if (fp = fopen (LST_NodeName (of), "wt")) {
		OutNodeType *on;
		
		DstFile = LST_NodeName (of);
		on = (OutNodeType *)LST_Head(&of->Format->OutList);

		while (!LST_IsEOList (on)) {
			if (on->IsBody) {
				PrintBody (fp, of->Format, &DefList);
			} else {
				char *s;
				++LineNo;
				s = SubstCFGArgs (LST_NodeName (on));
				UnEscStr (s);
				fprintf (fp, (s));
				fprintf (fp, "\n");
			}
			on = (OutNodeType*)LST_Next (on);
		}
		fclose (fp);
	} else {
		ErrMess ("Unable to open output file '%s'.\n", LST_NodeName (of));
	}

	return LST_Next (of);
}
} /* WriteOutputFile */

/*********************************************************************
 *
 * InterpretSource
 *
 * SYNOPSIS
 *		short	InterpretSource (LST_LIST *formatslist)
 *
 * PURPOSE
 *		To read the source file and write the output files.
 *
 * INPUT
 *
 *
 * AFFECTS
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
short	InterpretSource (LST_LIST *formatslist)
{
/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "InterpretSource";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/
{
	short	success = FALSE;
	LST_LIST	outfilelist;

	if (!formatslist) return FALSE;

	LST_InitList (&outfilelist);
	LST_InitList (&DefList);
	if (EIO_FileExists (SrcFile)) {
		FILE		*fp;
		if (fp = fopen (SrcFile, "r")) {
			static char	 Line[MAX_LINE_SIZE];
			char		*line;
			char		*args[MAX_ARGS + 1];
		
			LineNo = 0;
			while ((line = GetLine (Line, MAX_LINE_SIZE, fp)) != NULL) {
				KeyWordType kw = GetKeyWord (line);

				if (kw == KW_OUTPUT) {
					OutFileType *outfile;
					char *s = SubstCFGArgs (line);

					short argcount = argify (s, MAX_ARGS, args);
					if (argcount >= 2) {
						if (outfile = (OutFileType*)LST_CreateNode (sizeof (*outfile),
							args[1]))
						{
							char tag[EIO_MAXPATH];

							if ((argcount > 2  && strcpy (tag, args[2])) ||
								(EIO_fnsplit (args[1],NULL,NULL,tag) & 
									EIO_EXTENSION))
							{
								if (outfile->Format = GetFormat (formatslist, tag))
								{
									LST_AddTail (&outfilelist, outfile);
								} else {
									ErrMess ("Source file '%s': Line %d: "
										"No output format defined for output file type '%s'.\n",
										SrcFile, LineNo, args[1 + (argcount >2)]);
									LST_DeleteNode (outfile);
								}
							} else {
								ErrMess ("Source file '%s': Line %d: "
									"Missing output format tag and no extension on output filename.\n",
									SrcFile, LineNo);
							}

						} else {
							OutOfMemErr ("Source file '%s': creating output file node.\n",
								SrcFile, LineNo);
						}
					} else {
						ErrMess ("Syntax: Source file '%s': Line %d: missing filename argument to output command\n",
							SrcFile, LineNo);
					}
				} else { /* Is definition line */
					RuleNodeType *rulenode;
					short argcount = argify (line, MAX_ARGS, args);
					short i;

					if (rulenode = (RuleNodeType*)LST_CreateNode (sizeof (*rulenode),
						args[0]))
					{
						LST_InitList (&rulenode->ArgList);
						for (i = 1; i < argcount; i++) {
							LST_NODE *argnode;
							if (argnode = LST_CreateNode (sizeof (*argnode),
								args[i]))
							{
								LST_AddTail (&rulenode->ArgList, argnode);
							} else {
								OutOfMemErr ("CFG file '%s': creating rule arg node.\n",
									CFGFile, LineNo);
							}
						}
						LST_AddTail (&DefList, rulenode);
					} else {
						OutOfMemErr ("CFG file '%s': creating rule node.\n",
							CFGFile, LineNo);
					}
				}
			}
			success = TRUE;
			fclose (fp);
		} else {
			ErrMess ("Could not open source file '%s'.\n", SrcFile);
		}	
	} else {
		ErrMess ("Can't find source file '%s'.\n", SrcFile);
	}

	/** Write out each ouput file **/
	LST_OpOnList (&outfilelist, WriteOutputFile);

	return success;
}
} /* InterpretSource */

#define ARG_CFG		(newargs[0])
#define ARG_SOURCE		(newargs[1])
#define ARG_SARGS		(newargs[2])

char Usage[] = "Usage: Definer [switches] SOURCE [[ARGS] source args]\n";

ArgSpec Template[] = {
{	CHRKEYWORD_ARG,					"C",		"\t-C<cfg file> = Configuration file (default is DEFINER.CFG).\n", },
{	STANDARD_ARG | REQUIRED_ARG,	"SOURCE",	"\tSOURCE       = Source file that gets parsed.\n", },
{	STANDARD_ARG | MULTI_ARG,		"ARGS",		"\tARGS         = Arguments to source file.\n", },
{	0, NULL, NULL, },
};

int main (int argc, char **argv)
{
	char		**newargs = NULL;
	short		  success;

	fprintf (stderr, "Definer %s  Written by Juan M. Alvarado.\n", Version);

	if (argc > 1 && argv[1][0] != '?') {
		newargs = argparse (argc, argv, Template);
	}
	if (!newargs) {
		if (GlobalErrMsg) {
			fprintf (stderr, "%s\n", GlobalErrMsg);
		}
		printarghelp (Usage, Template);
		return EXIT_FAILURE;
	}

	if (ARG_CFG) {
		strcpy (CFGFile, ARG_CFG);
	}
	
	/* Get Current Directory */
	EIO_CurrentDir (CrntDirPath);

	strcpy (SrcFile, ARG_SOURCE);

	SArgs = (char **)ARG_SARGS;

	success = InterpretSource (PrintFormats(ReadConfig()));

	if (GlobalErr) {
		success = FALSE;
		ErrMess ("%s\n", GlobalErrMsg);
	}

	if (ErrorCount) {
		success = FALSE;
		fprintf (stderr, "Errors: %d\n", ErrorCount);
	}

	if (WarnCount) {
		fprintf (stderr, "Warnings: %d\n", WarnCount);
	}

	return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}
