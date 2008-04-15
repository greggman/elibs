/*
 * CUTBYTES.C
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 10/04/93
 *   MODIFIED : 10/05/93
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 *		Copyright (c) 1993-2008, Echidna
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
 *		
 *		Based on CUT by Dan Chang.  Switched to echidna to gain argparse
 *		and EL_atol support
 *
 * HISTORY
 *
*/

#include "platform.h"
#include "switches.h"

#include <echidna/eio.h>
#include <echidna/utils.h>
#include <echidna/eerrors.h>
#include <echidna/argparse.h>
#include <echidna/strings.h>

/**************************** C O N S T A N T S ***************************/

#define	BUF_SIZE	32768U

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

uint8	buffer[BUF_SIZE];

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


/******************************** TEMPLATE ********************************/

#define ARG_INFILE		(newargs[ 0])
#define ARG_OUTFILE		(newargs[ 1])
#define ARG_START		(newargs[ 2])
#define ARG_LENGTH		(newargs[ 3])

char Usage[] = "Usage: CUTBYTES INFILE OUTFILE Start [Length]\n";

ArgSpec Template[] = {
{STANDARD_ARG|REQUIRED_ARG,				"INFILE",	"\tINFILE  = Binary File to read\n", },
{STANDARD_ARG|REQUIRED_ARG,				"OUTFILE",	"\tOUTFILE = Binary File to write\n", },
{STANDARD_ARG|REQUIRED_ARG,				"START",	"\tSTART   = Number of Bytes To Skip\n", },
{STANDARD_ARG,							"LENGTH",	"\tLENGTH  = Number of Bytes To Copy\n", },
{0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
{
	char			**newargs;

	newargs = argparse (argc, argv, Template);

	if (!newargs) {
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		return EXIT_FAILURE;
	} else {

		int		infh;
		int		outfh;
		long	start;
		long	length;
		long	filelen;

		infh = EIO_ReadOpen (ARG_INFILE);
		if (infh == (-1))
		{
			FailMess ("Couldn't open %s\n", ARG_INFILE);
		}

		outfh = EIO_WriteOpen (ARG_OUTFILE);
		if (outfh == (-1))
		{
			FailMess ("Couldn't open %s\n", ARG_OUTFILE);
		}

		start   = EL_atol(ARG_START);

		filelen = EIO_FileLength (infh);
		if (start >= filelen)
		{
			FailMess ("Start [%ld] is past end-of-file [%ld]\n", start, filelen);
		}

		if (ARG_LENGTH)
		{
			length = EL_atol (ARG_LENGTH);
		}
		else
		{
			length = filelen - start;
		}

		if (length < 0)
		{
			FailMess ("Length less than 0!\n");
		}

		if (length + start > filelen)
		{
			length = filelen - start;

			WarnMess ("Length would pass end-of-file. Shortening to %ld\n", length);
		}

		if (EIO_Seek (infh, start, EIO_SEEK_BEGINNING) != start)
		{
			FailMess ("Problem skipping %ld bytes in %s\n", start, ARG_INFILE);
		}

//		EL_printf ("Cut %ld bytes from %s to %s starting from %ld\n", length, ARG_INFILE, ARG_OUTFILE, start);

		while (length)
		{
			long	size;
			long	bytes;

			size  = min (BUF_SIZE, length);
			bytes = EIO_Read (infh, buffer, size);
			if (bytes != size)
			{
				FailMess ("Problem reading file %s\n", ARG_INFILE);
			}

			if (EIO_Write (outfh, buffer, size) != size)
			{
				FailMess ("Problem writing file %s\n", ARG_OUTFILE);
			}

			length -= size;
		}

		EIO_Close (outfh);
		EIO_Close (infh);

	}

	if (GlobalErr) {
		EL_printf ("%s\n", GlobalErrMsg);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
