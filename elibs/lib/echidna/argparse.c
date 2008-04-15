/*************************************************************************
 *                                                                       *
 *                              ARGPARSE.C                               *
 *                                                                       *
 *************************************************************************

		Copyright (c) 1996-2008, Echidna

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
		Routines to parse command line arguments.

   PROGRAMMERS


   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/09/96 GAT: Created, base on Echidna code

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "echidna/eerrors.h"
#include "echidna/argparse.h"
#include "echidna/strings.h"
#include "echidna/ensure.h"
#include "echidna/eio.h"

/**************************** C O N S T A N T S ***************************/

#define TEST		0		/* Create Test Program						*/
#define DO_MULTI	1		/* Implement Multi-Args						*/

#define MAX_FILE_LINE_SIZE		512
#define MAX_FILE_ARGS_PER_LINE	64

/******************************* M A C R O S ******************************/

#if (__MSDOS__ && !__MSDOS32X__)
#define pointerdistance(end,start,len)	\
	{									\
		char huge32 *hs;				\
		char huge32 *he;				\
										\
		hs  = (char huge32 *)start;		\
		he  = (char huge32 *)end;		\
		len = (int)(he - hs);			\
	}
#else
#define pointerdistance(end,start,len)	(len) = ((end) - (start))
#endif

/***************************** R O U T I N E S ****************************/

/*------------------------------------------------------------------------*/
/**# MODULE:ARGPARSE_argify                                               */
/*------------------------------------------------------------------------*/

/**************************************************************************
 *
 * argify
 *
 * SYNOPSIS
 *		int argify (char *line, int num, char **args)
 *
 * PURPOSE
 *		Split line into 'num' CLI style arguements.  Stop at ';'
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
int argify (char *line, int maxargs, char **args)
{
	char	 c;
	int	 quote = FALSE;
	int	 count = 0;
	int	 esc   = FALSE;
	int	 start = FALSE;

	while ((c = *line) != '\0') {
		if (quote && !start) {
			start = TRUE;
			if (count < maxargs) {
				args[count] = line;
			}
			count++;
		}
		if ((!quote && (isspace (c) || c == ';')) || (quote && !esc && c == '"')) {
			*line = '\0';
			if (c == ';') {
				break;
			}
			quote = FALSE;
			start = FALSE;
		} else if (start && quote && !esc && c == '\\') {
			esc = TRUE;
		} else if (!quote && c == '"') {
			quote = TRUE;
		} else {
			if (!start) {
				if (count < maxargs) {
					args[count] = line;
				}
				count++;
				start = TRUE;
			}
			esc = FALSE;
		}
		line++;
	}
	args[count] = NULL;
	return count;

} /* argify */

/*------------------------------------------------------------------------*/
/**# MODULE:ARGPARSE_freeargs                                             */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * freeargs
 *
 * SYNOPSIS
 *		void freeargs(char **outv, char *template)
 *
 * PURPOSE
 *		frees any memory allocates by argparse() for a particular
 *		parsing. See argparse().
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
void freeargs(char **outv, ArgSpec *template)
{
	ArgSpec	*argspec;
	int		 ndx;

	/*
	 * for each arguement in the template.
	 *   if the arguement is of the multiargs type
	 *   free the memory used for it.
	 */

	argspec = template;
	ndx     = 0;
	while (argspec->ArgType) {
		if (outv[ndx] && (argspec->ArgType & MULTI_ARG) != 0)
		{
			if (argspec->ArgType & LIST_ARG)
			{
				LST_DeleteList (MULTI_ARGLINKEDLIST(outv[ndx]));
			}
			else
			{
				free (outv[ndx]);
			}
		}
		argspec++;
		ndx++;
	}
	free (outv);

} /* freeargs */

/*------------------------------------------------------------------------*/
/**# MODULE:ARGPARSE_argvark                                              */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * findpos
 *
 * SYNOPSIS
 *		int findpos(char **str, ArgSpec *template, int *type, int flag)
 *
 * PURPOSE
 *		finds the arguement number of 'str' in template and
 *		it's type.
 *
 * INPUT
 *		str      = pointer to keyword to search for
 *		template = pointer to arguement template, see argparse().
 *		type     = pointer to int variable for type.
 *
 * EFFECTS
 *		type equals type of arguement if match is found.
 *
 * RETURN VALUE
 *		pos = position of matched arguement (0-n)
 *			  or -1 if no match.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
typedef int (*StrNCmpFunc)(char *s1, char *s2, size_t len);

static int findpos(int *argc, char ***argv, ArgSpec *template, int *in_type, int *pos, char **arg)
{
	char	*e;
	char	*token;
	char	*str = **argv;
	char	 c0  = str[0];
	char	 c1  = str[1];
	int		 len;
	int		 inlen;
	int		 type = 0;
	int		 stype;
	int		 done;
	ArgSpec	*argspec;
	StrNCmpFunc		strnfunc;

	*arg	= str;
	inlen   = strlen (str);
	argspec = template;
	*pos    = 0;
	done    = FALSE;
	while (!done && argspec->ArgType) {
		type  = argspec->ArgType;
		stype = ARGTYPE (type);
		token = argspec->ArgTokens;
		while (!done && *token) {
			e = strchr (token, '=');
			if (!e) {
				e = token + strlen(token);
			}
			pointerdistance (e, token, len);

			/** If this is a single char arg then **/
			if (type & CHR_ARG) {
				if (( (type & CASE_SENSITIVE_ARG) && c1 == *token) ||
					(!(type & CASE_SENSITIVE_ARG) && tolower(c1) == tolower(*token))) {
					switch (stype) {
					case KEYWORD_ARG:
					case SWITCH_ARG:
						if (c0 == '/' || c0 == '-') {
							if (stype == KEYWORD_ARG) {
							 	*arg = &str[2];
								if (!**arg) {
									(*argc)--;
									(*argv)++;
									*arg = **argv;
								}
							}
							done = TRUE;
						}
						break;
					case TOGGLE_ARG:
						if (c0 == '-' || c0 == '+') {
							/** *arg = str; **/
							done = TRUE;
						}
						break;
					}
				}
			} else {
				strnfunc = (type & CASE_SENSITIVE_ARG) ? (StrNCmpFunc)(strncmp) : (StrNCmpFunc)(strnicmp);
				if (!strnfunc (token, str, (size_t)len) ||
                    (*token == '-' && stype == TOGGLE_ARG && *str == '+' && (!strnfunc (token+1, str+1, (size_t)len-1)))
                    ) {
					if (len == inlen) {
						if ((stype != SWITCH_ARG) && (stype != TOGGLE_ARG)) {
							(*argc)--;
							(*argv)++;
							*arg = **argv;
						}
						done = TRUE;
					} else if (str[len] == '=') {
						*arg = str + len + 1;
						if (!**arg) {
							(*argc)--;
							(*argv)++;
							*arg = **argv;
						}
						done = TRUE;
					}
				}
			}
			if (*e == '=') {
				e++;
			}
			token = e;
		}
		if (done) {
			*in_type = type;
		} else {
			(*pos)++;
			argspec++;
		}
	}

	if (!done) {
		*pos = -1;
	}

	if (!(type & MULTI_ARG) && !*arg) {
   		SetGlobalErr (ERR_GENERIC);
   		GEcatf ("Missing Argument after KEYWORD");
		return FALSE;
	}

	return TRUE;

} /* findpos */

/*********************************************************************
 *
 * multiargs
 *
 * SYNOPSIS
 *		char **multiargs(int *argc, char ***argv, ArgSpec *template)
 *
 * PURPOSE
 *		gather all multiargs.
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
static char **multiargs(
				char	*firstarg,
				int		*argc,
				char	***in_argv,
				ArgSpec *template,
				char	**prevMulti,
				int	list)
{
	char	**args;
	char	**hold;
	int		 count;
	char	**junkargv;
	int		 junkargc;
	int		 junktype;
	int		 junkpos;
	char	*junkarg;
	char	**oldargs;

	count    = 0;
	junkargc = *argc;
	junkargv = *in_argv;
	while (*argc - count > 0) {
		if (!findpos(&junkargc, &junkargv, template, &junktype, &junkpos, &junkarg)) {
			return NULL;
		}
		if (junkpos >= 0 || **junkargv == '@') {
			break;
		}
		junkargc--;
		junkargv++;
		count++;
	}

	
	if (!list)
	{
		//
		// Count old args
		//
		if (prevMulti)
		{
			oldargs = prevMulti;
			while (*oldargs)
			{
				count++;
				oldargs++;
			}
		}

		args = calloc ((size_t)count + 1 + 1, sizeof (char *));
		if (!args) {
   			SetGlobalErr (ERR_OUT_OF_MEMORY);
   			GEcatf ("Out of mem parsing multiargs");
			return NULL;
		}

		hold    = args;

		//
		// first copy old args
		//
		if (prevMulti) {
			oldargs = prevMulti;
			while (*oldargs)
			{
				*hold++ = *oldargs;
				count--;
				oldargs++;
			}
			free (prevMulti);
		}

		*hold++ = firstarg;
		for (; count > 0; count--) {
			*hold++ = **in_argv;
			(*in_argv)++;
			(*argc)--;
		}
	}
	else
	{
		LST_NODE	*nd;

		if (!prevMulti)
		{
			prevMulti = (char **)LST_CreateList (NULL);
			if (!prevMulti)
			{
   				SetGlobalErr (ERR_OUT_OF_MEMORY);
   				GEcatf ("\nOut Of mem paring multiargs list");
				return NULL;
			}
		}

		nd = LST_CreateNode (sizeof (LST_NODE), firstarg);
		if (!nd)
		{
   			SetGlobalErr (ERR_OUT_OF_MEMORY);
   			GEcatf ("\nOut Of mem paring multiargs list (3)");
			return NULL;
		}

		LST_AddTail (MULTI_ARGLINKEDLIST(prevMulti), nd);

		for (; count > 0; count--)
		{

			nd = LST_CreateNode (sizeof (LST_NODE), **in_argv);
			if (!nd)
			{
   				SetGlobalErr (ERR_OUT_OF_MEMORY);
   				GEcatf ("\nOut Of mem paring multiargs list (2)");
				return NULL;
			}

			LST_AddTail (MULTI_ARGLINKEDLIST(prevMulti), nd);
			(*in_argv)++;
			(*argc)--;
		}

		args = (char **)prevMulti;
	}


	return args;

} /* multiargs */

/*********************************************************************
 *
 * checkargs
 *
 * SYNOPSIS
 *		int checkargs (char **outv, ArgSpec *template)
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
static int checkargs (char **outv, ArgSpec *template, unsigned int flags)
{
	int		 ndx;
	ArgSpec	*argspec;

	ndx     = 0;
	argspec = template;
	while (argspec->ArgType)
	{
		if ((argspec->ArgType & flags) && !outv[ndx])
		{
			return FALSE;
		}
		ndx++;
		argspec++;
	}
	return TRUE;
} /* checkargs */

/*********************************************************************
 *
 * parseResponceFile
 *
 * SYNOPSIS
 *		int parseResponceFile (char *filename, char **outv, ArgSpec *template, int fail)
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
int parseResponceFile (char *filename, char **outv, ArgSpec *template, int fail, int *multi)
{
	char	 preline[MAX_FILE_LINE_SIZE];
	char	 line[MAX_FILE_LINE_SIZE];
	char	*args[MAX_FILE_ARGS_PER_LINE];
	FILE	*fp;
	int	 lineno = 0;
	int	 error;

	fp = fopen (filename, "r");
	if (!fp)
	{
   		SetGlobalErr (ERR_FILE_NOT_FOUND);
   		GEcatf1 ("Couldn't open responce file '%s'", filename);
		return FALSE;
	}

	while (fgets(preline, MAX_FILE_LINE_SIZE, fp))
	{
		int		 count;
		char	*s;

		lineno++;
		preline[strlen(preline) - 1] = '\0';	// remove end-of-line
		EIO_ExpandEVars(line, preline, MAX_FILE_LINE_SIZE);

		if (strlen (line))
		{
			s = dupstr (line);
			if (!s)
			{
   				SetGlobalErr (ERR_OUT_OF_MEMORY);
   				GEcatf2 ("Out of memory parsing responce file '%s' at line #%d", filename, lineno);
				error = TRUE;
				goto parseAbort;
			}

			count = argify (s, MAX_FILE_ARGS_PER_LINE, args);

			if (count)
			{
				char	**argv = args;

				if (!arganoid (outv, &count, &argv, template, fail, multi))
				{
				   GEcatf ("\n");
					error = TRUE;
					goto parseAbort;
				}
			}
			else
			{
				freestr (s);
			}
		}
	}

	error = ferror (fp);

parseAbort:
	fclose (fp);

	if (error)
	{
   		SetGlobalErr (ERR_READING_FILE);
   		GEcatf2 ("Error reading responce file '%s' at line #%d", filename, lineno);
		return FALSE;
	}

	return TRUE;

} /* parseResponceFile */

/*********************************************************************
 *
 * arganoid
 *
 * SYNOPSIS
 *		int arganoid (char **outv, int *in_argc, char ***in_argv, ArgSpec *template, int fail)
 *
 * PURPOSE
 *		Parse command line style arguments and create another
 *		'argv' type pointer array with the arguments in a
 *		specified order.
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
 *		argvark, argparse
 *	
*/
#define NEXTARG()	argv++; argc--;

int arganoid (char **outv, int *in_argc, char ***in_argv, ArgSpec *template, int fail, int *multi)
{
	int		 type;
	int		 pos;
	int		 ndx;
	int		 done;
	int		 finished;
	int		 argc;
	char	**argv;
	char	*arg;
	ArgSpec	*argspec;

	/** create local dereferenced copies of 'in_argc' and 'in_argv'	**/
	argc = *in_argc;
	argv = *in_argv;

	finished = FALSE;
	while (!finished && argc > 0) {
		if (!findpos (&argc, &argv, template, &type, &pos, &arg)) {
			goto argcleanup;
		}
		if (pos >= 0) {
			if (type & MULTI_ARG)
			{
				if (*arg == '@')
				{
					arg++;
					*multi = pos;
					goto kludgehere;
				}
				if (argc)
				{
					NEXTARG ();
					outv[pos] = (char *)multiargs(arg, &argc, &argv, template, (char **)outv[pos], type & LIST_ARG);
					if (!outv[pos])
					{
						goto argcleanup;
					}
				}
				//
				// if last arg then member which multiarg we were doing
				// so if there are more args on another line (rsp file)
				// they will collect at this multiarg.
				//
				*multi = (!argc || **argv == '@') ? pos : (-1);
			}
			else
			{
				*multi = (-1);
				switch (ARGTYPE(type))
				{
				case STANDARD_ARG:
				case KEYWORD_ARG:
					if (*arg == '@')
					{
						arg++;
						*multi = pos;
						goto kludgehere;
					}
					outv[pos] = arg;
					NEXTARG ();
					break;
				case SWITCH_ARG:
					outv[pos] = (char *)TRUE;
					NEXTARG ();
					break;
				case TOGGLE_ARG:
					outv[pos] = (char *)*arg;
					NEXTARG ();
					break;
				default:
   					SetGlobalErr (ERR_GENERIC);
   					GEcatf ("Malformed Template");
					goto argcleanup;
				}
			}
		}
		else if (**argv == '@')
		{
			arg = *argv + 1;
kludgehere:
			NEXTARG ();
			if (!parseResponceFile (arg, outv, template, fail, multi))
			{
				goto argcleanup;
			}
		}
		else
		{
			/** fill in possible args. **/
			ndx     = 0;
			argspec = template;
			done    = FALSE;
			while (!done && argspec->ArgType)
			{
				if ((*multi < 0 && !outv[ndx]) || (*multi == ndx))
				{
					if (ARGTYPE(argspec->ArgType) == STANDARD_ARG && (argspec->ArgType & MULTI_ARG))
					{
						arg = *argv;
						NEXTARG ();
		   				outv[ndx] = (char *)multiargs(arg, &argc, &argv, template, (char **)outv[ndx], argspec->ArgType & LIST_ARG);
		   				if (!outv[ndx])
						{
		   					goto argcleanup;
		   				}
						//
						// if last arg then member which multiarg we were doing
						// so if there are more args on another line (rsp file)
						// they will collect at this multiarg.
						//
						*multi = (!argc || **argv == '@') ? ndx : (-1);
		   				done = TRUE;
					}
					else
					{
						switch (ARGTYPE (argspec->ArgType))
						{
						case STANDARD_ARG:
		   					outv[ndx] = *argv;
		   					NEXTARG ();
		   					done = TRUE;
		   					break;
						default:
							break;
						}
					}
				}
				ndx++;
				argspec++;
			}
			if (!done)
			{
				if (!fail)
				{
   					SetGlobalErr (ERR_GENERIC);
   					GEcatf ("Bad positional arguement?");
					goto argcleanup;
				}
				else
				{
					finished = TRUE;
				}
			}
		}
		if (fail && checkargs(outv, template, EXIT_ARG))
		{
			finished = TRUE;
		}
		if (argc <= 0)
		{
			finished = TRUE;
		}
	}

	if (argc < 0)
	{
   		SetGlobalErr (ERR_GENERIC);
   		GEcatf ("parsed too many args! I don't know why.  Bugs in argparse()");
		goto argcleanup;
	}

	/** make sure required args exist. **/
	if (!checkargs (outv, template, REQUIRED_ARG))
	{
   		SetGlobalErr (ERR_GENERIC);
   		GEcatf ("Required Argument Missing");
		goto argcleanup;
	}

	*in_argc = argc;
	*in_argv = argv;

	return TRUE;

argcleanup:
	return FALSE;

} /* arganoid */

/*********************************************************************
 *
 * argvark
 *
 * SYNOPSIS
 *		char **argvark(int *argc, char ***argv, ArgSpec *template, int fail)
 *
 * PURPOSE
 *		create another 'argv' type pointer array for arguments
 *		can then pass it to arganoid for parsing.
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
 *		argparse, arganoid
*/
char **argvark(int *in_argc, char ***in_argv, ArgSpec *template, int fail)
{
	int		 count;
	int		 none = FALSE;
	int		 multi = (-1);
	char	**outv = NULL;
	ArgSpec	*argspec;

	/** count total args. **/
	count   = 0;
	argspec = template;
	while (argspec->ArgType)
	{
		count++;
		argspec++;
	}

	if (!count)
	{
		if (*in_argc)
		{
   			SetGlobalErr (ERR_GENERIC);
   			GEcatf ("no arguments allowed");
			goto argcleanup;
		}
		else
		{
			none  = TRUE;
			count = 1;
		}
	}

	/** allocate memory for arg pointers **/
	outv = calloc ((size_t)count, sizeof (char *));
	if (!outv)
	{
   		SetGlobalErr (ERR_OUT_OF_MEMORY);
   		GEcatf ("Out of mem for arguement pointers");
		goto argcleanup;
	}
	if (none)
	{
		return outv;
	}

	if (arganoid (outv, in_argc, in_argv, template, fail, &multi))
	{
		return outv;
	}

argcleanup:
	if (outv)	freeargs (outv, template);

	return NULL;

} /* argvark */

/*------------------------------------------------------------------------*/
/**# MODULE:ARGPARSE_argparse                                             */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * argparse
 *
 * SYNOPSIS
 *		char **argparse(int argc, char **argv, ArgSpec *template)
 *
 * PURPOSE
 *		Parse command line style arguments and create another
 *		'argv' type pointer array with the arguments in a
 *		specified order.
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
 *		argvark, arganoid
*/

char **argparse(int in_argc, char **in_argv, ArgSpec *template)
{
	in_argc--;
	in_argv++;

	return argvark(&in_argc, &in_argv, template, (int)FALSE);

} /* argparse */

/*------------------------------------------------------------------------*/
/**# MODULE:ARGPARSE_printarghelp                                         */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * printarghelp
 *
 * SYNOPSIS
 *		void printarghelp (char *usage, ArgSpec *template)
 *
 * PURPOSE
 *		print help associated with 'template'.
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
void printarghelp (char *usage, ArgSpec *template)
{
	printf ("%s", usage);

	while (template->ArgType)
	{
		printf ("%s", template->ArgHelp);
		template++;
	}

} /* printarghelp */

/*------------------------------------------------------------------------*/
/**# IGNORE                                                               */
/*------------------------------------------------------------------------*/

#if TEST
/****************************** E X A M P L E *****************************/

#define ARG_FROM	(newargs[0])
#define ARG_DIR		(newargs[1])
#define ARG_PAT		(newargs[2])
#define ARG_KEYS	(newargs[3])
#define ARG_O		(newargs[4])
#define ARG_P		(newargs[5])
#define ARG_N		(newargs[6])
#define ARG_POOF	(newargs[7])
#define ARG_LOTS	(newargs[8])

char Usage[] = "Usage: argparse FROM/a,DIR,PAT/k,KEYS/s,O/s,P/t,N/k,LOTS...,POOF/s\n";

ArgSpec Template[] = {
{	STANDARD_ARG | REQUIRED_ARG,	"FROM", "\tFROM = FROMFILE\n", },
{	STANDARD_ARG,					"DIR",	"\tDIR  = DIRECTORY to scan\n", },
{	KEYWORD_ARG,					"PAT",	"\tPAT  = File Pattern to match\n", },
{	SWITCH_ARG,						"KEYS",	"\tKEYS = Turn on Keys\n", },
{	CHRSWITCH_ARG,					"O",	"\t-O   = Generate Object File\n", },
{	CHRTOGGLE_ARG,					"P",	"\t+P   = Purge mem (-p)\n", },
{	CHRKEYWORD_ARG,					"N",	"\t-N<Name> = Name to save as\n", },
{	SWITCH_ARG,						"POOF",	"\tPOOF = Flash Screen\n", },
{	STANDARD_ARG | MULTI_ARG | LIST_ARG,		"LOTS",	"\tLOTS = Other Files...\n", },
{	0, NULL, NULL, },
};

#define PS(str)		((str)?(str):"<NULL>")
#define PB(val)		((val)?"TRUE":"FALSE")
#define PT(val)		(((int)(val) == '+') ? "+" : (((int)(val) == '-') ? "-" : "NOT"))

/***************************** Test of ArgVark ****************************/
#if 0
void main (int argc, char **argv)
{
	int		 i;
	char	**newargv;
	char	**lots;
	int		 myargc;
	char	**myargv;
	int		 bad;

	for (i = 0; i < argc; i++) {
		printf ("arg %2d : '%s'\n", i, argv[i]);
	}

	myargc = argc - 1;
	myargv = argv + 1;
	bad    = FALSE;
	do {
		newargv = argvark (&myargc, &myargv, Template, TRUE);
		if (!newargv) {
			printf ("%s\n", GlobalErrMsg);
			printarghelp (Usage, Template);
			bad = TRUE;
		} else {
			printf ("------------------------------\n");
			printf ("From           = '%s'\n", PS(newargv[ARG_FROM]));
			printf ("Dir            = '%s'\n", PS(newargv[ARG_DIR]));
			printf ("Pat            = '%s'\n", PS(newargv[ARG_PAT]));
			printf ("Keys           = '%s'\n", PB(newargv[ARG_KEYS]));
			printf ("O              = '%s'\n", PB(newargv[ARG_O]));
			printf ("P              = '%s'\n", PT(newargv[ARG_P]));
			printf ("N              = '%s'\n", PS(newargv[ARG_N]));
			lots = (char **)newargv[ARG_LOTS];
			if (lots) {
				i = 1;
				while (*lots) {
					printf ("Lots %-3d   = '%s'\n", i++, *lots);
					lots++;
				}
			}
			printf ("Poof           = '%s'\n", PB(newargv[ARG_POOF]));
			freeargs (newargv, Template);
		}
	} while (myargc && !bad);
}
#endif

/**************************** Test of ArgParse ****************************/
#if 1
void main (int argc, char **argv)
{
	int		 i;
	char	**newargs;

	for (i = 0; i < argc; i++) {
		printf ("arg %2d : '%s'\n", i, argv[i]);
	}

	newargs = argparse (argc, argv, Template);
	if (!newargs) {
		printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
	} else {
		printf ("-----------------------------\n");
		printf ("From           = '%s'\n", PS(ARG_FROM));
		printf ("Dir            = '%s'\n", PS(ARG_DIR));
		printf ("Pat            = '%s'\n", PS(ARG_PAT));
		printf ("Keys           = '%s'\n", PB(SWITCH_VALUE(ARG_KEYS)));
		printf ("O              = '%s'\n", PB(SWITCH_VALUE(ARG_O)));
		printf ("P              = '%s'\n", PT(SWITCH_VALUE(ARG_P)));
		printf ("N              = '%s'\n", PS(ARG_N));

		#if 0
		{
			char	**lots;

			lots = MULTI_ARGLIST(ARG_LOTS);
			if (lots) {
				i = 1;
				while (*lots) {
					printf ("Lots %-3d   = '%s'\n", i++, *lots);
					lots++;
				}
			}
		}
		#else
		{
			LST_LIST	*lots;
			LST_NODE	*nd;

			lots = MULTI_ARGLINKEDLIST (ARG_LOTS);
			if (lots)
			{
				i = 1;
				nd = LST_Head (lots);
				while (!LST_IsEOList (nd))
				{
					printf ("Lots %-3d   = '%s'\n", i++, LST_NodeName (nd));
					nd = LST_Next (nd);
				}
			}
		}
		#endif
		printf ("Poof           = '%s'\n", PB(SWITCH_VALUE(ARG_POOF)));
	}
}
#endif
#endif
