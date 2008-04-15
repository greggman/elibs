/*
 * MAKESCAN.c
 *
 * PROGRAMMER : Gregg A. Tavares 
 *    VERSION : 00.000
 *    CREATED : 02/11/90 
 *   MODIFIED : 04/27/90
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w-----
 *	 E C H I D N A
 *	--------------
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
 *		Program to create link files from a makefile.
 *
 *		Language:
 *
 *
 *		KEYWORD:
 *			_Open FILENAME							: Opens a file for output.
 *			_Names NAME [RULE] [REPEATRULE] [END]	: Specfies a ?
 *			_Close									: Close a file.
 *			_Print STRING							: Print String to file
 *		#											: End of rule.
 *
 *		Example:
 *
 *		PROGRAM:
 *			_Open "linkfile.lnk"
 *			_Names "PDEP" "%s" "+\n%s"
 *			_Close
 *			_Open "libs.lnk"
 *			_Names "LDEP" "%e<LIB>%s" "+\n%e<LIB>%s"
 *			_Close
 *
 * HISTORY
 *
*/
#include <echidna/platform.h>
#include "switches.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <echidna/eio.h>
#include <echidna/eerrors.h>
#include <echidna/argparse.h>
#include <echidna/listapi.h>
#include <echidna/strings.h>

/**************************** C O N S T A N T S ***************************/
#if _EL_PLAT_AMIGA__
#define CONFIGFILE	"makescan.acfg"
#define MAKEFILE	"makefile.ami"
#define PROGRAMFILE	"makescan"
#endif
#if _EL_PLAT_WIN32__
#define CONFIGFILE	"makescan.cfg"
#define MAKEFILE	"makefile.mak"
#define PROGRAMFILE	"makescan.exe"
#endif

#define	CMD_OPEN	1
#define CMD_NAMES	2
#define CMD_PRINT	3
#define CMD_CLOSE	4

#define MAXLINE		256
#define TOTALARGS	5

/***************************** T Y P E D E F S ****************************/

typedef int (*linefuncptr)(FILE *fp, char *linebuff);

typedef struct {
	LST_NODE	 Node;
	int  		 Command;
	char		*Arg1;
	char		*Arg2;
	char		*Arg3;
} Statement;

typedef struct {
	LST_NODE	 Node;
	LST_LIST	 Statements;
} ConfigType;

typedef struct {
	LST_NODE	 Node;
	LST_LIST	 Names;
	int  		 EnvFlag;
} MakeMacro;

typedef struct {
	LST_NODE	 Node;
	MakeMacro	*Macro;
	int  		 Delim;
} NameType;

typedef struct {
	char		*Keyword;
	ArgSpec		*Template;
} CommandType;

typedef struct {
	LST_NODE	 Node;
	NameType	*Father;
} StackType;

/********************* G L O B A L   V A R I A B L E S ********************/
int  			DebugSwitch	= FALSE;
int  			LineNumber	= 0;
int  			StartFlag	= FALSE;
int  			CurrentType;
int  			IgnoreSlash = FALSE;
linefuncptr		LineFunc;

/***************** F O R W A R D   D E C L A R A T I O N S ****************/
extern NameType *PrintName (NameType *name);
extern int  	 FPrintName (NameType *name, FILE *fp);

/******************************* T A B L E S ******************************/
#define OPENARG_FILENAME	0
ArgSpec OpenTemplate[] = {
	STANDARD_ARG | REQUIRED_ARG,	"FILENAME",		NULL,
	0, NULL, NULL,
};

#define PRINTARG_STRING		0
ArgSpec PrintTemplate[] = {
	STANDARD_ARG | REQUIRED_ARG,	"STRING",		NULL,
	0, NULL, NULL,
};

#define NAMESARG_NAME		0
#define NAMESARG_RULE		1
#define NAMESARG_REPEAT		2
#define NAMESARG_END		3
ArgSpec NamesTemplate[] = {
	STANDARD_ARG | REQUIRED_ARG,	"NAME",			NULL,
	STANDARD_ARG,					"RULE",			NULL,
	STANDARD_ARG,					"REPEATRULE",	NULL,
	STANDARD_ARG,					"END",			NULL,
	0, NULL, NULL,
};

CommandType Commands[] = {
	"_Open",	&OpenTemplate[0],
	"_Names",	&NamesTemplate[0],
	"_Print",	&PrintTemplate[0],
	"_Close",	NULL,
	NULL,		NULL,
};

/***************************** R O U T I N E S ****************************/

/**************************************************************************
 *
 * GetLine 
 *
 * SYNOPSIS
 *		int   GetLine (FILE *fp, char *linebuff)
 *
 * PURPOSE
 *		Get a line from specfied file.  Skip blank lines and
 *		lines that start with '*'.  Also remove end-of-line.
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
int   GetLine (FILE *fp, char *linebuff)
{	while (fgets (linebuff, MAXLINE, fp)) {
		LineNumber++;
		if (strlen (linebuff) && *linebuff != '*') {
			return 1;
		}
	}
	if (feof (fp)) {
		return 0;
	}
	return (-1);

} /* GetLine  */

/*********************************************************************
 *
 * FreeMacro
 *
 * SYNOPSIS
 *		void FreeMacro (MacroType *mac)
 *
 * PURPOSE
 *		Free a macro?
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
void FreeMacro (MakeMacro *mac)
{
	LST_NODE	*name;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "FreeMacro";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	while ((name = LST_RemTail (&mac->Names)) != NULL) {
		LST_DeleteNode (name);
	}
	LST_DeleteNode (mac);

} /* FreeMacro */



/**************************************************************************
 *
 * ReadConfigurationFile
 *
 * SYNOPSIS
 *		LST_LIST *ReadConfigurationFile (char *filename)
 *
 * PURPOSE
 *		Read specified configuration file (or default if not specifed)
 *		Read all rules and return list.
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
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
LST_LIST *ReadConfigurationFile (char *filename)
{
	char		 configfilebuff[EIO_MAXPATH];
	char		 linebuff[MAXLINE];
	char		*line;
	char		*configfile;
	char		*args[TOTALARGS];
	char		**newargs;
	char		**junkargs;
	int			 junkcount;
	int  		 status;
	int  		 count;
	int  		 found;
	int  		 cmdnum;
	int  		 len;
	int  		 getit;
	int  		 error	= FALSE;
	FILE		*fp		= NULL;
	LST_LIST	*list	= NULL;
	ConfigType	*cf;
	CommandType	*cmd;
	Statement	*st;

	/** Find Config File **/
	if (filename) {
		configfile = filename;
	} else {
		if (!EIO_FindFile (CONFIGFILE, PROGRAMFILE, configfilebuff)) {
			printf ("Config file not found in path!\n");
			goto rcfcleanup;
		}
		configfile = configfilebuff;
	}

	/** Open Config File **/
	fp = fopen (configfile, "r");
	if (!fp) {
		SetGlobalErr (ERR_FILE_NOT_FOUND);
		GEprintf1 ("couldn't open configfile '%s'", configfile);
		goto rcfcleanup;
	}

	/** Read Config File **/
	list = LST_CreateList (NULL);
	if (!list) {
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("couldn't allocate list\n");
		goto rcfcleanup;
	}

	while ((status = GetLine (fp, linebuff)) > 0) {
		line = TrimWhiteSpace (linebuff);
		len  = strlen (line);
		if (len != 0) {
			if (line[len - 1] != ':') {
				printf ("Error in config file:Missing starting keyword at line %d\n", LineNumber);
				error = TRUE;
			} else {
				line[len - 1] = '\0';
				strupr (line);
				cf = (ConfigType*)LST_CreateNode (sizeof (ConfigType), line);
				LST_AddTail (list, cf);
				LST_InitList (&cf->Statements);
				while ((status = GetLine (fp, linebuff)) > 0) {
					if (linebuff[0] == '#') {
						break;
					}
					line = TrimWhiteSpace (linebuff);
					len  = strlen (line);
					if (len) {
						count  = argify (line, TOTALARGS, args);
						found  = FALSE;
						cmd    = Commands;
						cmdnum = 1;
						while (cmd->Keyword && !found) {
							if (!stricmp (args[0], cmd->Keyword)) {
								found     = TRUE;
								junkcount = count - 1;
								junkargs  = &args[1];
								getit     = TRUE;
								newargs   = NULL;
								if (cmd->Template) {
									newargs = argvark (&junkcount, &junkargs, cmd->Template, FALSE);
									if (!newargs) {
										printf ("Error in config file:%s (at line %d)\n", GlobalErrMsg, LineNumber);
										error = TRUE;
										getit = FALSE;
									}
								}
								if (getit) {
									switch (cmdnum) {
									case CMD_OPEN:
										st = (Statement*)LST_CreateNode (sizeof (Statement), NULL);
										if (!st) {
											printf ("OOM: (1) creating config statement at line %d\n", LineNumber);
											error = TRUE;
										} else {
											LST_AddTail (&cf->Statements, st);
											st->Command = CMD_OPEN;
											st->Arg1    = dupstr (newargs[OPENARG_FILENAME]);
											if (!st->Arg1) {
												printf ("OOM: duplicating _Open filename\n");
												error = TRUE;
											}
										}
										break;
									case CMD_PRINT:
										st = (Statement*)LST_CreateNode (sizeof (Statement), newargs[PRINTARG_STRING]);
										if (!st) {
											printf ("OOM: (2) creating config statement at line %d\n", LineNumber);
											error = TRUE;
										} else {
											LST_AddTail (&cf->Statements, st);
											st->Command = CMD_PRINT;
										}
										break;
									case CMD_NAMES:
										strupr (newargs[NAMESARG_NAME]);
										st = (Statement*)LST_CreateNode (sizeof (Statement), newargs[NAMESARG_NAME]);
										if (!st) {
											printf ("OOM: (3) creating config statement at line %d\n", LineNumber);
											error = TRUE;
										} else {
											LST_AddTail (&cf->Statements, st);
											st->Command = CMD_NAMES;
											if (newargs[NAMESARG_RULE]) {
												st->Arg1 = dupstr (newargs[NAMESARG_RULE]);
												if (!st->Arg1) {
													printf ("OOM: duplicating _Names Rule Arg\n");
													error = TRUE;
												}
											}
											if (newargs[NAMESARG_REPEAT]) {
												st->Arg2 = dupstr (newargs[NAMESARG_REPEAT]);
												if (!st->Arg2) {
													printf ("OOM: duplicating _Names Repeat arg\n");
													error = TRUE;
												}
											}
											if (newargs[NAMESARG_END]) {
												st->Arg3 = dupstr (newargs[NAMESARG_END]);
												if (!st->Arg3) {
													printf ("OOM: duplicating _Names End arg\n");
													error = TRUE;
												}
											}
										}
										break;
									case CMD_CLOSE:
										st = (Statement*)LST_CreateNode (sizeof (Statement), NULL);
										if (!st) {
											printf ("OOM: (2) creating config statement at line %d\n", LineNumber);
											error = TRUE;
										} else {
											LST_AddTail (&cf->Statements, st);
											st->Command = CMD_CLOSE;
										}
										break;
									}
									if (newargs) {
										freeargs (newargs, cmd->Template);
									}
								}
							}
							cmdnum++;
							cmd++;
						}
						if (!found) {
							printf ("Error in config file:Bad keyword '%s' at line %d\n", args[0], LineNumber);
							error = TRUE;
						}
					}
				}
			}
		}
	}

	/** Exit if error **/
	if (status || error) {
		goto rcfcleanup;
	}

	return list;

rcfcleanup:
	if (fp)			fclose (fp);
	return NULL;

} /* ReadConfigurationFile */

/**************************************************************************
 *
 * PrintConfigList 
 *
 * SYNOPSIS
 *		void PrintConfigList (LST_LIST *list)
 *
 * PURPOSE
 *		Print ConfigList (for Debugging)
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
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void PrintConfigList (LST_LIST *list)
{
	ConfigType	*cf;
	Statement	*st;

	printf ("------Config Rules------\n");
	cf = (ConfigType*)LST_Head (list);
	while (!LST_EndOfList (cf)) {
		printf ("%s:\n", LST_NodeName(cf));
		st = (Statement*)LST_Head (&cf->Statements);
		while (!LST_EndOfList (st)) {
			switch (st->Command) {
			case CMD_OPEN:
				printf ("\t_Open");
				if (st->Arg1) {
					printf (" \"%s\"", st->Arg1);
				}
				printf ("\n");
				break;
			case CMD_PRINT:
				printf ("\t_Print \"%s\"\n", LST_NodeName(st));
				break;
			case CMD_NAMES:
				printf ("\t_Names \"%s\"", LST_NodeName(st));
				if (st->Arg1) {
					printf (" RULE \"%s\"", st->Arg1);
				}
				if (st->Arg2) {
					printf (" REPEATRULE \"%s\"", st->Arg2);
				}
				if (st->Arg3) {
					printf (" END \"%s\"", st->Arg3);
				}
				printf ("\n");
				break;
			case CMD_CLOSE:
				printf ("\t_Close\n");
				break;
			default:
				printf ("***\tERROR: Unknown command (%d)\n", st->Command);
				break;
			}
			st = (Statement*)LST_Next (st);
		}
		cf = (ConfigType*)LST_Next (cf);
	}

} /* PrintConfigList  */

/*********************************************************************
 *
 * GetNextEnv
 *
 * SYNOPSIS
 *		int   GetNextEnv (FILE *fp, char *linebuff)
 *
 * PURPOSE
 *		get environment variable
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
BOOL		 GNEstarting = TRUE;
DirTracker	*GNEdt;
int   GetNextEnv (FILE *fp, char *linebuff)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetNextEnv";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if (GNEstarting) {
		GNEstarting = FALSE;
		
		GNEdt = EIO_GetFirstEnv ();
		if (!GNEdt) {
			*linebuff = '\0';
			return (-1);
		}
	} else {
		if (!EIO_GetNextEnv (GNEdt)) {
			return (-1);
		}
	}
	if (GNEdt->Status) {
		strcpy (linebuff, GNEdt->Path);
	} else {
		CurrentType = 0;
		IgnoreSlash = FALSE;
		LineFunc = GetLine;
		return GetLine (fp, linebuff);
	}
	return TRUE;

} /* GetNextEnv */


/**************************************************************************
 *
 * GetMakeMacros 
 *
 * SYNOPSIS
 *		LST_LIST *GetMakeMacros (char *filename)
 *
 * PURPOSE
 *		Parse a make file and gather it's macros and their tokens into
 *		lists.
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
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
LST_LIST *GetMakeMacros (char *filename)
{
	FILE		*fp;
	LST_LIST	*mlist;
	MakeMacro	*macro;
	MakeMacro	*sub;
	MakeMacro	*oldmac;
	NameType	*name;
	NameType 	*last;
	int  		 status;
	int  		 cont;
	int  		 len;
	int  		 error = FALSE;
	char		 linebuff[MAXLINE];
	char		 macroId[MAXLINE];
	char		*line;
	char		*s;
	char		*w;
	char		 c;

	fp = fopen (filename, "r");
	if (!fp) {
		SetGlobalErr (ERR_FILE_NOT_FOUND);
		GEprintf1 ("Couldn't open file '%s' as makefile", filename);
		return NULL;
	}

	mlist = LST_CreateList (NULL);
	if (!mlist) {
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("OOM: allocating macro list");
		return NULL;
	}

	CurrentType = 1;
	IgnoreSlash = TRUE;
	LineFunc = GetNextEnv;
	LineNumber = 0;
	while ((status = LineFunc (fp, linebuff)) > 0) {
		line = TrimWhiteSpace (linebuff);
		len  = strlen (line);
		s    = macroId;
		cont = FALSE;
		if (len && *line != '#') {
			while (*line && (isalnum(*line) || *line == '_')) {
				*s++ = *line++;
			}
			*s = '\0';
			while (*line && isspace(*line)) {
				line++;
			}
			if (*line == '=') {
				line++;
				strupr (macroId);
				oldmac = (MakeMacro*)LST_FindName (mlist, macroId);
				if (oldmac) {
					if (oldmac->EnvFlag) {
						LST_Remove (oldmac);
						FreeMacro (oldmac);
					} else {
						printf ("ERROR:Duplicate Macro '%s' at line %d\n", macroId, LineNumber);
						error = TRUE;
					}
				}
				macro = (MakeMacro*)LST_CreateNode (sizeof (MakeMacro), macroId);
				if (!macro) {
					SetGlobalErr (ERR_OUT_OF_MEMORY);
					GEprintf ("OOM: macro\n");
					return NULL;
				}
				macro->EnvFlag = CurrentType;
				LST_InitList (&macro->Names);
				LST_AddTail (mlist, macro);
				last = NULL;
				s = strtok (line, " \t");
				if (s) {
					do {
						s = TrimWhiteSpace (s);
						len = strlen (s);
						if (len && s[len - 1] == '\\' && !IgnoreSlash) {
							len--;
							s[len] = '\0';
							cont   = TRUE;
						}
						if (len) {
							while (*s) {
								w = s;
								while (*s && !(*s == '$' && s[1] == '(')) {
									s++;
								}
								c  = *s;
								*s = 0;
								if (strlen (w)) {
									name = (NameType*)LST_CreateNode (sizeof (NameType), w);
									if (!name) {
											SetGlobalErr (ERR_OUT_OF_MEMORY);
											GEprintf ("OOM: (2) finishing macro substitution");
											return NULL;
									}
									LST_AddTail (&macro->Names, name);
								}
								*s = c;
								if (*s == '$' && s[1] == '(') {
									s += 2;
									w = s;
									s = strchr (s, ')');
									if (!s) {
										printf ("ERROR:Macro missing ')' at line %d\n", LineNumber);
										error = TRUE;
									} else {
										*s = 0;
											s++;
										strupr (w);
										sub = (MakeMacro*)LST_FindName (mlist, w);
										if (!sub) {
											printf ("Undefined macro '%s' in line %d\n", w, LineNumber);
											error = TRUE;
										} else {
											name = (NameType*)LST_CreateNode (sizeof (NameType), NULL);
											if (!name) {
													SetGlobalErr (ERR_OUT_OF_MEMORY);
													GEprintf ("OOM: (4) finishing macro substitution");
													return NULL;
											}
											name->Macro = sub;
											LST_AddTail (&macro->Names, name);
										}
									}
								}
							}
							name = (NameType*)LST_CreateNode (sizeof (NameType), NULL);
							if (!name) {
									SetGlobalErr (ERR_OUT_OF_MEMORY);
									GEprintf ("OOM: (6) adding delimiter");
									return NULL;
							}
							name->Delim = TRUE;
							last = name;
							LST_AddTail (&macro->Names, name);
						}
						s = strtok (NULL, " \t");
						if (!s && cont) {
							do {
								status = LineFunc (fp, linebuff);
							} while (status > 0 && *linebuff == '#');
							if (status == 0) {
								SetGlobalErr (ERR_GENERIC);
								GEprintf ("EOF: unterminated macro");
								return NULL;
							} else if (status < 0) {
								SetGlobalErr (ERR_GENERIC);
								GEprintf1 ("I/O ERROR: reading file '%s'", filename);
								return NULL;
							}
							line = TrimWhiteSpace (linebuff);
							s = strtok (line, " \t");
						}
						cont = FALSE;
					} while (s);
					if (last && last->Delim) {
						LST_Remove (last);
						LST_DeleteNode (last);
					}
				}
			}
		}
	}

	/** Exit if error **/
	if (status || error) {
		return NULL;
	}

	return mlist;

} /* GetMakeMacros  */

/**************************************************************************
 *
 * PrintMacro
 *
 * SYNOPSIS
 *		void PrintMacro (MakeMacro *macro)
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
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void PrintMacro (MakeMacro *macro)
{
	NameType	*name;

	name = (NameType*)LST_Head (&macro->Names);
	while (!LST_EndOfList (name)) {
		name = PrintName (name);
	}

} /* PrintMacro */

/**************************************************************************
 *
 * PrintName
 *
 * SYNOPSIS
 *		NameType *PrintName (NameType *name)
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
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
NameType *PrintName (NameType *name)
{
	while (!LST_EndOfList (name)) {
		if (LST_NodeName(name)) {
			if (!StartFlag) {
				printf (" \\\n\t");
				StartFlag = TRUE;
			}
			printf ("%s", LST_NodeName(name));
		}
		if (name->Macro) {
			PrintMacro (name->Macro);
		}
		name = (NameType*)LST_Next (name);
		if (!LST_EndOfList(name) && name->Delim) {
			StartFlag = FALSE;
			break;
		}
	}
	return name;

} /* PrintName */

/**************************************************************************
 *
 * PrintMacros 
 *
 * SYNOPSIS
 *		void PrintMacros (LST_LIST *mlist)
 *
 * PURPOSE
 *		Print all the macros
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
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void PrintMacros (LST_LIST *mlist)
{
	MakeMacro	*macro;

	printf ("------Macros------\n");
	macro = (MakeMacro*)LST_Head (mlist);
	while (!LST_EndOfList (macro)) {
		printf ("'%s' = ", LST_NodeName(macro));
		StartFlag = FALSE;
		PrintMacro (macro);
		printf ("\n");
		macro = (MakeMacro*)LST_Next (macro);
	}
	printf ("------End of Macros------\n");

} /* PrintMacros  */

/**************************************************************************
 *
 * PrintEscString 
 *
 * SYNOPSIS
 *		void PrintEscString (char *str, FILE *fp)
 *
 * PURPOSE
 *		Print a string with 'C' style escape sequences (ie '\n' '\t'...);
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
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void PrintEscString (char *s, FILE *fp)
{
	int		 c;
	char	 digit[10];
	int  	 digits = 0;
	int  	 esc = FALSE;
	int  	 hex = FALSE;
	int  	 oct = FALSE;

	while (*s) {
		if (oct) {
			if (*s >= '0' && *s <= '7') {
				digit[digits++] = *s++;
				if (digits == 3 || *s < '0' || *s > '7') {
					oct = FALSE;
					digit[digits] = '\0';
					sscanf (digit, "%o", &c);
					putc (c, fp);
				}
				continue;
			} else {
				oct = FALSE;
				digit[digits] = '\0';
				sscanf (digit, "%o", &c);
				putc (c, fp);
			}
		} else if (hex) {
			if (isxdigit(*s)) {
				digit[digits++] = *s++;
				if (digits == 2 || !isxdigit(*s)) {
					hex = FALSE;
					digit[digits] = '\0';
					sscanf (digit, "%x", &c);
					putc (c, fp);
				}
				continue;
			} else {
				hex = FALSE;
				digit[digits] = '\0';
				sscanf (digit, "%x", &c);
				putc (c, fp);
			}
		}
		if (!esc && *s == '\\') {
			esc = TRUE;
			s++;
		} else if (esc) {
			esc = FALSE;
			switch (*s) {
			case 'b': putc ('\b', fp); break;
			case 'f': putc ('\f', fp); break;
			case 'n': putc ('\n', fp); break;
			case 'r': putc ('\r', fp); break;
			case 't': putc ('\t', fp); break;
			case 'v': putc ('\v', fp); break;
			case 'x': hex = TRUE; digits = 0; break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
					  oct = TRUE; digits = 1; digit[0] = *s; break;
			default:  putc (*s, fp); break;
			}
			s++;
		} else {
			putc (*s, fp);
			s++;
		}
	}
} /* PrintEscString  */

/**************************************************************************
 *
 * FPrintMacro
 *
 * SYNOPSIS
 *		int   FPrintMacro (MakeMacro *macro, FILE *fp)
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
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int   FPrintMacro (MakeMacro *macro, FILE *fp)
{
	NameType	*name;

	name = (NameType*)LST_Head (&macro->Names);
	if (!FPrintName (name, fp)) {
		return FALSE;
	}

	return TRUE;

} /* FPrintMacro */

/**************************************************************************
 *
 * FPrintName
 *
 * SYNOPSIS
 *		int   FPrintName (NameType *name, FILE *fp)
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
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int   FPrintName (NameType *name, FILE *fp)
{
	while (!LST_EndOfList (name)) {
		if (LST_NodeName(name)) {
			fprintf (fp, "%s", LST_NodeName(name));
		}
		if (name->Macro) {
			if (!FPrintMacro (name->Macro, fp)) {
				return FALSE;
			}
		}
		name = (NameType*)LST_Next (name);
		if (!LST_EndOfList(name) && name->Delim) {
			return FALSE;
		}
	}
	return TRUE;

} /* FPrintName */

/**************************************************************************
 *
 * ApplyRule 
 *
 * SYNOPSIS
 *		int   ApplyRule (NameType *name, char *rule, FILE *fp)
 *
 * PURPOSE
 *		Substitute node->Name in rule for '%s' and write to file.
 *		Also Substitute environment vars for '%e<var>' and translate
 *		escape charactes.
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
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int   ApplyRule (NameType *name, char *rule, FILE *fp)
{
	char	 c;
	char	*s;
	char	*env;
	char	 work[MAXLINE];
	int  	 esc = FALSE;

	if (strlen (rule) >= MAXLINE) {
		printf ("ERROR:rule too large (change MAXLINE)\n");
		return FALSE;
	}
	strcpy (work, rule);
	rule = work;

	while (*rule) {
		s = rule;
		while (*s && !(!esc && *s == '%')) {
			if (!esc && *s == '\\') {
				esc = TRUE;
				s++;
			} else if (esc) {
				esc = FALSE;
				s++;
			} else {
				s++;
			}
		}
		esc = FALSE;
		c  = *s;
		*s = '\0';
		PrintEscString (rule, fp);
		*s = c;
		if (*s) {
			s++;
			if (tolower(*s) == 's') {
				s++;
				FPrintName (name, fp);
			} else if (tolower(*s) == 'e') {
				s++;
				if (*s != '<') {
					printf ("ERROR:missing '<' on %%e parameter in rule\n");
				} else {
					s++;
					rule = s;
					while (*s && *s != '>') {
						s++;
					}
					if (!*s) {
						printf ("ERROR:missing '>' on %%e parameter in rule\n");
					} else {
						*s = '\0';
						s++;
						env = getenv (rule);
						if (env) {
							fprintf (fp, "%s", env);
						}
					}
				}
			}
		}
		rule = s;
	}

	return TRUE;

} /* ApplyRule  */

/**************************************************************************
 *
 * CreateFiles 
 *
 * SYNOPSIS
 *		int   CreateFiles (LST_LIST *clist, char *filename)
 *
 * PURPOSE
 *		Follow specified rule and create files.
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
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int   CreateFiles (ConfigType *cf, char *filename)
{
	LST_LIST	*macrolist;
	LST_LIST	*stacklist;
	StackType	*stack;
	MakeMacro	*macro;
	NameType	*name;
	Statement	*st;
	FILE		*fp = NULL;
	int  		 firstflag;
	int  		 error = FALSE;
	int  		 stacksize = 0;

	if (!filename) {
		filename = MAKEFILE;
	}
	macrolist = GetMakeMacros (filename);
	if (!macrolist) {
		return FALSE;
	}

	stacklist = LST_CreateList (NULL);
	if (!stacklist) {
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("OOM: couldn't allocate stack list");
		return FALSE;
	}

	if (DebugSwitch) {
		PrintMacros (macrolist);
	}

	st = (Statement*)LST_Head (&cf->Statements);
	while (!LST_EndOfList (st)) {
		switch (st->Command) {
		case CMD_OPEN:
			if (fp) {
				fclose (fp);
			}
			fp = fopen (st->Arg1, "w");
			if (!fp) {
				SetGlobalErr (ERR_GENERIC);
				GEprintf1 ("Couldn't open file '%s' for writing", st->Arg1);
				return FALSE;
			}
			break;
		case CMD_PRINT:
			if (!fp) {
				printf ("ERROR:'_Print' command issued with no file opened\n");
				error = TRUE;
				break;
			}
			PrintEscString (LST_NodeName(st), fp);
			break;
		case CMD_NAMES:
			macro = (MakeMacro*)LST_FindName (macrolist, LST_NodeName(st));
			if (!macro) {
				SetGlobalErr (ERR_GENERIC);
				GEprintf2 ("'%s' macro not found in file '%s'", LST_NodeName(st), filename);
				return FALSE;
			}
			if (!fp) {
				printf ("ERROR:'_Names' command issued with no file opened\n");
				error = TRUE;
				break;
			}
			firstflag = TRUE;
			name      = (NameType*)LST_Head (&macro->Names);
			while (!LST_EndOfList (name)) {
				if (firstflag && st->Arg1) {
					firstflag = FALSE;
					if (!ApplyRule (name, st->Arg1, fp)) {
						error = TRUE;
					}
				} else if (st->Arg2) {
					if (!ApplyRule (name, st->Arg2, fp)) {
						error = TRUE;
					}
				}
				while (!LST_EndOfList (name)) {
					if (name->Macro) {
						stack = (StackType*)LST_CreateNode (sizeof (StackType), NULL);
						if (!stack) {
							SetGlobalErr (ERR_OUT_OF_MEMORY);
							GEprintf ("OOM: couldn't allocate stack entry");
							return FALSE;
						}
						stack->Father = name;
						LST_AddTail (stacklist, stack);
						stacksize++;
						name = (NameType*)LST_Head (&name->Macro->Names);
					} else if (name->Delim) {
						name = (NameType*)LST_Next (name);
						break;
					} else {
						name = (NameType*)LST_Next (name);
						while (stacksize && LST_EndOfList (name)) {
							stack = (StackType*)LST_RemTail (stacklist);
							if (stack) {
								name = (NameType*)LST_Next (stack->Father);
								LST_DeleteNode (stack);
								stacksize--;
							}
						}
					}
				}
			}
			if (st->Arg3) {
				PrintEscString (st->Arg3, fp);
			}
			break;
		case CMD_CLOSE:
			if (fp) {
				fclose (fp);
				fp = NULL;
			}
			break;
		}
		st = (Statement*)LST_Next (st);
	}

	return (error ? FALSE : TRUE);
} /* CreateFiles  */

/******************************** TEMPLATE ********************************/
#define ARG_RULE		0
#define ARG_CONFIG		1
#define ARG_MAKEFILE	2
#define ARG_DEBUG		3

char Usage[] = "Usage: MakeScan [RULE] [CONFIG] [MAKEFILE]\n";

ArgSpec Template[] = {
	STANDARD_ARG,	"RULE",		"\tRULE     = rule in config file to follow\n",
	STANDARD_ARG,	"CONFIG",	"\tCONFIG   = config file to use 'default = " CONFIGFILE "'\n",
	STANDARD_ARG,	"MAKEFILE",	"\tMAKEFILE = makefile to scan 'default = " MAKEFILE "'\n",
	SWITCH_ARG,		"DEBUG",	"",
	0, NULL, NULL,
};

/********************************* M A I N ********************************/
int main (
	int argc,
	char **argv
) {
	char		**newargs = NULL;
	LST_LIST	*configlist;
	ConfigType	*config;
	int  		 status = TRUE;

	printf ("MakeScan (c) Echidna 1990\n");

	if (!(argc == 2 && argv[1][0] == '?' && argv[1][1] == '\0')) {
		newargs = argparse (argc, argv, Template);
	}
	if (!newargs) {
		if (GlobalErrMsg) {
			printf ("%s\n", GlobalErrMsg);
		}
		printarghelp (Usage, Template);
	} else {
		DebugSwitch = (int  )((long)newargs[ARG_DEBUG]);
		if ((configlist = ReadConfigurationFile (newargs[ARG_CONFIG]))) {
			if (DebugSwitch) {
				PrintConfigList (configlist);
			}
			if (newargs[ARG_RULE]) {
				strupr (newargs[ARG_RULE]);
				if (!(config = (ConfigType*)LST_FindName (configlist, newargs[ARG_RULE]))) {
					SetGlobalErr (ERR_GENERIC);
					GEprintf1 ("No such rule '%s'", newargs[ARG_RULE]);
					status = FALSE;
				}
			} else {
				config = (ConfigType*)LST_Head (configlist);
			}
			if (status) {
				if (!CreateFiles (config, newargs[ARG_MAKEFILE])) {
					status = FALSE;
				}
			}
		} else {
			status = FALSE;
		}

		if (GlobalErr) {
			printf ("%s\n", GlobalErrMsg);
			status = FALSE;
		}
	}
	return (status ? 0 : 20);
}

