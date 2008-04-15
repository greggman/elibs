/*************************************************************************
 *                                                                       *
 *                           COPYFRAMES.CPP                              *
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
		Copy files in alphabetical order

   PROGRAMMERS


   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/15/96 : Created.

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include <echidna\ensure.h>

#include <echidna\argparse.h>
#include <echidna\eerrors.h>
#include <echidna\eio.h>
#include <echidna\strings.h>

/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/


/************************** P R O T O T Y P E S **************************/


/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/


/**************************** R O U T I N E S ****************************/

/*********************************************************************
 *
 * EIO_CopyFile
 *
 * SYNOPSIS
 *		int	 EIO_CopyFile (const char* from, const char* to)
 *
 * PURPOSE
 *		Copies file from TO file to including creation date,
 *		file attributes and file comments.
 *
 * INPUT
 *		from	: pointer to source filename
 *		to		: pointer to dest filename
 *
 * RETURN VALUE
 *		FALSE = failure
 *
 * EXAMPLE
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <stdio.h>
 *	
 *		int main (void)
 *		{
 *			if (EIO_CopyFile ("C:\\AUTOEXEC.BAT", "D:\\AUTOEXEC.BAT")) {
 *	
 *				EL_printf ("AUTOEXEC.BAT copyied from C:\\ to D:\\\n");
 *			}
 *	
 *			return 0;
 *		}
 *	
 *	
 * HISTORY
 *
 *
 * SEE ALSO
 *		EIO_GetFileDate, EIO_SetFileDate
 *		EIO_GetFileComment, EIO_SetFileComment
 *		EIO_GetFileAttrib, EIO_SetFileAttrib
 *	
*/
#define CBUF_SIZE	16384
int	 CopyFile (const char* from, const char* to)
{
	{
		uint8	*buf = NULL;
		int		 in  = -1;
		int		 out = -1;
		long	 bytes;
		int		 status = FALSE;
		
		buf = (uint8 *)malloc (CBUF_SIZE);
		if (!buf) {
			SetGlobalErr (ERR_OUT_OF_MEMORY);
			GEprintf ("EIO_CopyFile:OOM copy buffer");
			goto cfcleanup;
		}
	
		in = EIO_ReadOpen (from);
		if (in == (-1)) {
			SetGlobalErr (ERR_GENERIC);
			GEprintf1 ("EIO_CopyFile:Couldn't open source file '%s'", from);
			goto cfcleanup;
		}
	
		out = EIO_WriteOpen (to);
		if (out == (-1)) {
			SetGlobalErr (ERR_GENERIC);
			GEprintf1 ("EIO_CopyFile:Couldn't open destination file '%s'", to);
			goto cfcleanup;
		}
	
		while ((bytes = EIO_Read (in, buf, CBUF_SIZE)) > 0) {
//			EL_printf ("copying %ld\n", bytes);
			if (EIO_Write (out, buf, bytes) == (-1)) {
				SetGlobalErr (ERR_GENERIC);
				GEprintf1 ("EIO_CopyFile:Error Writing to file '%s'", to);
				goto cfcleanup;
			}
		}
	
		if (bytes < 0) {
			SetGlobalErr (ERR_GENERIC);
			GEprintf1 ("EIO_CopyFile:Error Reading from file '%s'", from);
			goto cfcleanup;
		}
	
		close (out);	out = -1;
		close (in);		in  = -1;

		#if 0
		{
			FileDateType	fdt;
	
			if (!EIO_GetFileDate (from, &fdt)) {
				goto cfcleanup;
			}
	
			if (!EIO_SetFileDate (to, &fdt)) {
				goto cfcleanup;
			}
		}
	
		{
			FileAttribType	fat;
	
			if (!EIO_GetFileAttrib (from, &fat)) {
				goto cfcleanup;
			}
	
			if (!EIO_SetFileAttrib (to, &fat)) {
				goto cfcleanup;
			}
	
		}
	
		{
			FileCommentType	fct;
	
			if (!EIO_GetFileComment (from, &fct)) {
				goto cfcleanup;
			}
	
			if (!EIO_SetFileComment (to, &fct)) {
				goto cfcleanup;
			}
		}
		#endif
	
		status = TRUE;
	
	cfcleanup:
		if (buf) free (buf);
		if (out != -1) close (out);
		if (in  != -1) close (in);
		return status;
	}
	
} /* EIO_CopyFile */


#define ARG_INFILES		(newargs[ 0])
#define ARG_DESTDIR		(newargs[ 1])

char Usage[] = "Usage: copyframes INFILES DESTDIR\n";

ArgSpec Template[] = {
{STANDARD_ARG|REQUIRED_ARG,				"INFILES",	"\tINFILES = files to copy (like mycrap*.tga)\n", },
{STANDARD_ARG|REQUIRED_ARG,				"DESTDIR",	"\tDESTDIR = destination directory\n", },
{0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
BEGINFUNCMAIN(main)
{
	char			**newargs;

	newargs = argparse (argc, argv, Template);

	if (!newargs)
	{
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		return EXIT_FAILURE;
	}
	else
	{
		LST_LIST	*fileList;
		
		fileList = EIO_GetFileList (ARG_INFILES, FALSE, FALSE);
		if (fileList)
		{
			// sort
			
			LST_LIST	sortListX;
			LST_LIST*	pSortList = &sortListX;
			LST_NODE*	pND;
			
			LST_InitList (pSortList);
			
			while ((pND = LST_RemTail (fileList)) != NULL)
			{
				LST_NODE*	pSortNode;
				
				pSortNode = LST_Head (pSortList);
				while (!LST_EndOfList (pSortNode))
				{
					if (stricmp(LST_NodeName (pND), LST_NodeName (pSortNode)) < 0)
					{
						break;
					}
					pSortNode = LST_Next (pSortNode);
				}
				
				LST_InsertBefore (pSortNode, pND);
			}
			
			// copy
			
			{
				LST_NODE*	pSortNode;
				
				pSortNode = LST_Head (pSortList);
				while (!LST_EndOfList (pSortNode))
				{
					char	dest[EIO_MAXPATH];
					
					EIO_fnmerge (dest, ARG_DESTDIR, EIO_Filename (LST_NodeName (pSortNode)), NULL);
					
					EL_printf ("Copying '%s' -> '%s'\n", LST_NodeName (pSortNode), dest);
					
					if (!CopyFile (LST_NodeName (pSortNode), dest))
					{
						break;
					}
					
					pSortNode = LST_Next (pSortNode);
				}
			}
			
		}
	}

	if (GlobalErr) {
		EL_printf ("%s\n", GlobalErrMsg);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
ENDFUNCMAIN(main)