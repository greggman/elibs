/*************************************************************************
 *                                                                       *
 *                       CHECK4PROJECTS.CPP                              *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION
		Scan a directory full of sub directories and figure out
		if there is a project inside each sub directory for example
		by checking if there is a makefile in each subdirectory.

   PROGRAMMERS


   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/15/96 : Created.

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna\ensure.h"

#include "echidna\argparse.h"
#include "echidna\eerrors.h"
#include "echidna\eio.h"
#include "echidna\strings.h"

/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/


/************************** P R O T O T Y P E S **************************/


/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/


/**************************** R O U T I N E S ****************************/

#define ARG_VERBOSE		(newargs[ 0])
#define ARG_DIRSPEC		(newargs[ 1])
#define ARG_OUTSTRING	(newargs[ 2]) 
#define	ARG_OTHERFILES	(newargs[ 3])

char Usage[] = "Usage: check4projects DIRSPEC OUTSTRING\n";

ArgSpec Template[] = {
{SWITCH_ARG,							"-V",			"\t-V         = Verbose\n", },
{STANDARD_ARG|REQUIRED_ARG,				"DIRSPEC",		"\tDIRSPEC    = Spec for file and directories (eg, m:\\work\\*.c)\n", },
{STANDARD_ARG|REQUIRED_ARG,				"OUTSTRING",	"\tOUTSTRING  = string to output for each project found\n"
														"\t               (eg. c:\\work\\foo.c)\n"
														"\t               %%n = name      'foo'\n"
														"\t               %%p = path      'c:\\work\\'\n"
														"\t               %%e = ext       '.c'\n"
														"\t               %%f = filename  'foo.c'\n"
														"\t               %%s = filespec  'c:\\work\\foo.c\n"
														, },
{KEYWORD_ARG|MULTI_ARG,					"-O",			"\t-O <fname> = another file that must also exist\n"
														"\t               (eg. itsalevel.tag)\n", },
{0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
BEGINFUNCMAIN(main)
{
	char			**newargs;
	int				  fVerbose;

	newargs = argparse (argc, argv, Template);

	if (!newargs)
	{
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		return EXIT_FAILURE;
	}
	else
	{
		char		 dirPath[EIO_MAXPATH];
		char		 ext[EIO_MAXPATH];
		char		 dirSpec[EIO_MAXPATH];
		DirTracker	*dt;
		int			 aokay = TRUE;
		
		fVerbose = SWITCH_VALUE(ARG_VERBOSE);

		EIO_fnsplit (ARG_DIRSPEC, dirPath, NULL, ext);
		EIO_fnmerge (dirSpec, dirPath, DIRMATCHALL, NULL);
		
		if (fVerbose) EL_printf ("Scanning '%s'\n", dirSpec);
		
		dt = EIO_GetFirstFile (dirSpec, TRUE);
		if (dt)
		{
			while (aokay && dt->Status)
			{
				if (dt->IsDir)
				{
					char	prjPath[EIO_MAXPATH];
					char	prjSpec[EIO_MAXPATH];

					EIO_fnmerge (prjPath, dirPath, dt->Path, NULL);
					EIO_fnmerge (prjSpec, prjPath, dt->Path, ext);

					if (fVerbose) EL_printf ("Checking '%s'", prjSpec);
					if (EIO_FileExists (prjSpec))
					{
						BOOL	fExists = TRUE;
						char** files;
						
						files = MULTI_ARGLIST(ARG_OTHERFILES);
						if (files)
						{
							while (*files)
							{
								char	otherSpec[EIO_MAXPATH];
								
								EIO_fnmerge (otherSpec, prjPath, *files, NULL);
								
								if (!EIO_FileExists (otherSpec))
								{
									fExists = FALSE;
									break;
								}
								files++;
							}
						}
						
						if (fExists)
						{
							if (fVerbose) EL_printf ("-=>Exists<=-\n");
	
							// print out string
							{
								char*	s;
								
								s = ARG_OUTSTRING;
								while (*s)
								{
									if (*s == '%')
									{
										s++;
										switch (*s)
										{
										case 'n':
											fputs (EIO_Name(prjSpec), stdout);
											break;
										case 'p':
											fputs (EIO_Path(prjSpec), stdout);
											break;
										case 'e':
											fputs (EIO_Ext(prjSpec), stdout);
											break;
										case 'f':
											fputs (EIO_Filename(prjSpec), stdout);
											break;
										case 's':
											fputs (prjSpec, stdout);
											break;
										default:
											putc (*s, stdout);
											break;
										}
									}
									else
									{
										putc (*s, stdout);
									}
									s++;
								}
	
								EL_printf ("\n");
							}
						}
					}
					else
					{
						if (fVerbose) EL_printf ("\n");
					}
				}
				aokay = EIO_GetNextFile (dt);
			}
			EIO_FreeDirTracker (dt);
		}
		if (!aokay || !dt)
		{
			EL_printf ("Error reading directory\n");
		}
	}

	if (GlobalErr) {
		EL_printf ("%s\n", GlobalErrMsg);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
ENDFUNCMAIN(main)