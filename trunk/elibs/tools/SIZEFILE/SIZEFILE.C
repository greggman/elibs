/*
 * SIZEFILE.C
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 08/27/92
 *   MODIFIED : 08/27/92
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 *		Copyright (c) 1992-2008, Echidna
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
 *		Write the size of a file to another file.
 *
 * HISTORY
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include <echidna/eerrors.h>
#include <echidna/argparse.h>
#include <echidna/eio.h>

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/******************************** TEMPLATE ********************************/

#define ARG_INFILE			(newargs[ 0])
#define ARG_OUTFILE			(newargs[ 1])

char Usage[] = "Usage: Sizefile INFILE OUTFILE [switches...]\n";

ArgSpec Template[] = {
{	STANDARD_ARG | REQUIRED_ARG,	"INFILE",	"\tINFILE        = Data file\n", },
{	STANDARD_ARG | REQUIRED_ARG,	"OUTFILE",	"\tOUTFILE       = Size File\n", },
{	0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
{
	char			**newargs;
	long			 size;
	int				 fh;

	newargs = argparse (argc, argv, Template);

	if (!newargs) {
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		return EXIT_FAILURE;
	} else {

		fh = EIO_ReadOpen (ARG_INFILE);
		if (fh == (-1))
		{
			EL_printf ("Error, couldn't open infile '%s'\n", ARG_INFILE);
	 		return EXIT_FAILURE;
		}
		size = EIO_FileLength (fh);
		EIO_Close (fh);
			
		fh = EIO_WriteOpen (ARG_OUTFILE);
		if (fh == (-1))
		{
			EL_printf ("Error, couldn't open outfile '%s'\n", ARG_OUTFILE);
	 		return EXIT_FAILURE;
		}
				
		EL_printf ("Data file size = %ld\n", size);

		EIO_Write (fh, &size, sizeof (size));
		EIO_Close (fh);

	}

	return EXIT_SUCCESS;
}

