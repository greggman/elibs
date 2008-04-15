/*************************************************************************
 *                                                                       *
 *                       EGCHECKSIZE.CPP                                 *
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
		Check the size of a bunch of files and return error if they
		are larger than a given size.

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
#include "echidna\checkglu.h"
#include "echidna\strings.h"

/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/


/************************** P R O T O T Y P E S **************************/


/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/


/**************************** R O U T I N E S ****************************/

#define ARG_VERBOSE		(newargs[ 0])
#define	ARG_RECURSE		(newargs[ 1])
#define ARG_SIZE		(newargs[ 2])
#define ARG_FILES		(newargs[ 3]) 

char Usage[] =	"Usage: BGCheckSize SIZE FILE1 FILE2...\n"
				"     wildcards ok (eg. f*.*)\n"
				;

ArgSpec Template[] = {
{SWITCH_ARG,							"-V",		"\t-V    = Verbose\n", },
{SWITCH_ARG,							"-R",		"\t-R    = Recurse subdirectories\n", },
{STANDARD_ARG|REQUIRED_ARG,				"SIZE",		"\tSIZE  = max size\n", },
{STANDARD_ARG|MULTI_ARG|REQUIRED_ARG,	"FILES",	"\tFILES = file to compare\n", },
{0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
BEGINFUNCMAIN(main)
{
	char			**newargs;
	int				  fVerbose;
	int				  success = FALSE;

	newargs = argparse (argc, argv, Template);

	if (!newargs)
	{
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		RETURN EXIT_FAILURE;
	}
	else
	{
		long	totalSize = 0;
		long	maxSize;
		BOOL	fRecurseDirs;
		
		maxSize = EL_atol (ARG_SIZE);
		
		fVerbose     = SWITCH_VALUE(ARG_VERBOSE);
		fRecurseDirs = SWITCH_VALUE(ARG_RECURSE);
		
		{
			char** files;
			
			files = MULTI_ARGLIST(ARG_FILES);
			
			while (*files)
			{
				LST_LIST*	fileList;
				
				if (fVerbose)
				{
					EL_printf ("Getting file list for '%s'\n", *files);
				}
				
				fileList = EIO_GetFileList (*files, fRecurseDirs, FALSE);
				if (!fileList)
				{
					FailMess ("Couldn't get filelist for '%s'\n", *files);
				}
				else
				{
					LST_NODE*	nd;
					
					nd = LST_Head (fileList);
					while (!LST_EndOfList (nd))
					{
						int	fh;
						long fileSize;
						
						
						fh = CHK_ReadOpen (LST_NodeName(nd));
						fileSize = CHK_FileLength (fh);
						CHK_Close (fh);
						
						totalSize += fileSize;
						
						if (fVerbose)
						{
							EL_printf ("File '%s' : size %d\n", LST_NodeName(nd), fileSize);
						}
						
						nd = LST_Next (nd);
					}
				}
				files++;
			}
		}
		
		if (fVerbose)
		{
			EL_printf ("Total Size = %d\n", totalSize);
		}
		
		if (totalSize > maxSize)
		{
			EL_printf (
				"Files are %d bytes too big!\n"
				"    Max Size = %d\n"
				"Current Size = %d\n"
				, totalSize - maxSize
				, maxSize
				, totalSize
				);
		}
		else
		{
			success = TRUE;
		}
	}
	
	if (GlobalErr) {
		EL_printf ("%s\n", GlobalErrMsg);
		return EXIT_FAILURE;
	}

	return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}
ENDFUNCMAIN(main)


