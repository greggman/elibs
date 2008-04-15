/*
 * HEXDUMP.C
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 05/09/90
 *   MODIFIED : 03/04/93
 *       TABS : 05 09
 *
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
 *	Print a binary file in HEX with lots of useful options.
 *
 *
 * HISTORY
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <echidna/eerrors.h>
#include <echidna/argparse.h>
#include <echidna/eio.h>

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/

UINT8	*BigBuffer;
UINT8	 SmallBuffer[4];

short	Data;
short	Size = 1;
short	Flip;
short	ByteCount = 16;
UINT32	Address;

/******************************* M A C R O S ******************************/


/******************************* T A B L E S ******************************/


/***************************** R O U T I N E S ****************************/

/******************************** TEMPLATE ********************************/

#define ARG_INFILE			(newargs[ 0])
#define ARG_LONG			(newargs[ 1])
#define ARG_WORD			(newargs[ 2])
#define ARG_FLIP			(newargs[ 3])
#define ARG_COUNT			(newargs[ 4])
#define ARG_DATA			(newargs[ 5])

char Usage[] = "Usage: HEXDUMP INFILE [switches...]\n";

ArgSpec Template[] = {
{	STANDARD_ARG | REQUIRED_ARG,	"INFILE",	"\tINFILE  = Binary file to dump\n", },
{	CHRSWITCH_ARG,					"L",		"\t-L      = Print as LONG\n", },
{	CHRSWITCH_ARG,					"W",		"\t-W      = Print as WORD\n", },
{	CHRSWITCH_ARG,					"F",		"\t-F      = Use 68000 conventions\n", },
{	CHRKEYWORD_ARG,					"C",		"\t-C<num> = Count of bytes per line\n", },
{	CHRKEYWORD_ARG,					"D",		"\t-D<num> = Format for Data\n"
												"\t          0 = Dump (default)\n"
												"\t          1 = C Style\n"
												"\t          2 = DB,DW,DL ($) ASM Style\n"
												"\t          3 = DB,DW,DL (h) ASM Style\n"
												"\t          4 = DC.B,DC.W,DC.L ASM Style\n"
												, },
{	0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
{
	char			**newargs;
	FILE			*fp;

	newargs = argparse (argc, argv, Template);

	if (!newargs) {
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		return EXIT_FAILURE;
	} else {

		if (ARG_COUNT) { ByteCount = atoi (ARG_COUNT); }
		if (ARG_DATA)  { Data = atoi (ARG_DATA); }

		BigBuffer = malloc (ByteCount);
		if (!BigBuffer) {
			EL_printf ("Out Of Memory allocating byte buffer\n");
			return EXIT_FAILURE;
		}
		Flip = SWITCH_VALUE(ARG_FLIP);
		Size = (SWITCH_VALUE(ARG_LONG)) ? 4 : Size;
		Size = (SWITCH_VALUE(ARG_WORD)) ? 2 : Size;

		fp = fopen (ARG_INFILE, "rb");
		if (!fp) {
			EL_printf ("Couldn't open file '%s'\n", ARG_INFILE);
			return EXIT_FAILURE;
		}

	//
	// do the dump
	//

		{
			short	count;
			short	sizecount;
			short	dump;
			short	start;
			short	done;
			int		byte;

			sizecount = 0;
			count     = 0;
			dump      = FALSE;
			start     = TRUE;
			done      = FALSE;
			while (!done) {
			
				byte = fgetc (fp);
				if (byte == EOF) {
					done = TRUE;
					dump = TRUE;
					ByteCount = count;
				}

				if (!done) {

					BigBuffer[count]       = byte;
					SmallBuffer[sizecount] = byte;

					sizecount++;
					if (sizecount == Size) {
						dump = TRUE;
					}

					count++;
					if (count == ByteCount) {
						dump = TRUE;
					}
				}

				if (count) {

					if (start) {
						switch (Data) {
						case 0:
							EL_printf ("%08x", Address);
							break;
						case 1:
							EL_printf ("\t");
							break;
						case 2:
							switch (Size) {
							case 1:
								EL_printf ("\tDB\t");
								break;
							case 2:
								EL_printf ("\tDW\t");
								break;
							case 4:
								EL_printf ("\tDL\t");
								break;
							}
							break;
						case 3:
							switch (Size) {
							case 1:
								EL_printf ("\tDB\t");
								break;
							case 2:
								EL_printf ("\tDW\t");
								break;
							case 4:
								EL_printf ("\tDL\t");
								break;
							}
							break;
						case 4:
							switch (Size) {
							case 1:
								EL_printf ("\tDC.B\t");
								break;
							case 2:
								EL_printf ("\tDC.W\t");
								break;
							case 4:
								EL_printf ("\tDC.L\t");
								break;
							}
							break;
						}
						start = FALSE;
					}

					if (dump) {

						dump      = FALSE;
						sizecount = 0;

						if (!Flip) {
							UINT8	 temp;

							if (Size == 2) {
								temp = SmallBuffer[0];
								SmallBuffer[0] = SmallBuffer[1];
								SmallBuffer[1] = temp;
							} else if (Size == 4) {
								temp = SmallBuffer[0];
								SmallBuffer[0] = SmallBuffer[3];
								SmallBuffer[3] = temp;
								temp = SmallBuffer[1];
								SmallBuffer[1] = SmallBuffer[2];
								SmallBuffer[2] = temp;
							}
						}
						switch (Data) {
						case 0:
							EL_printf (" ");
							break;
						case 1:
							EL_printf ("0x");
							break;
						case 2:
							EL_printf ("$");
							break;
						case 3:
							EL_printf ("0");
							break;
						case 4:
							EL_printf ("$");
							break;
						}
						{
							short	i;

							for (i = 0; i < Size; i++) {
								EL_printf ("%02x", SmallBuffer[i]);
							}
						}

						if (count != ByteCount) {
							switch (Data) {
							case 0:
								break;
							case 1:
								EL_printf (",");
								break;
							case 2:
								EL_printf (",");
								break;
							case 3:
								EL_printf ("h,");
								break;
							case 4:
								EL_printf (",");
								break;
							}
						}

						SmallBuffer[0] =
						SmallBuffer[1] =
						SmallBuffer[2] =
						SmallBuffer[3] = 0;

					}

					if (count == ByteCount) {
						short	i;

						count = 0;
						start = TRUE;

						switch (Data) {
						case 0:
							EL_printf ("  ");
							for (i = 0; i < ByteCount; i++) {
								if (isprint(((char)(BigBuffer[i])))) {
									putchar (BigBuffer[i]);
								} else {
									putchar ('.');
								}
							}
							EL_printf ("\n");
							break;
						case 1:
							EL_printf (",\n");
							break;
						case 2:
							EL_printf ("\n");
							break;
						case 3:
							EL_printf ("h\n");
							break;
						case 4:
							EL_printf ("\n");
							break;
						}
					}
				}

				Address++;
			}
		}
	}

	if (GlobalErr) {
		EL_printf ("%s\n", GlobalErrMsg);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

