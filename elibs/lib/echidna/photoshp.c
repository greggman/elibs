/*
 * PHOTOSHP.C
 *
 *  COPYRIGHT : 1994 Echidna.
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 09/13/94
 *   MODIFIED : 09/15/94
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oO///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 *		Copyright (c) 1996-2008, Echidna
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
 *
 * HISTORY
 *
*/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include "echidna/eerrors.h"
#include "echidna/readgfx.h"
#include "echidna/memfile.h"
#include "photoshp.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/

typedef struct
{
	uint32	signature;
	uint16	version;
	uint8	reserved[6];
	uint16	channels;
	uint32	rows;
	uint32	columns;
	uint16	depth;
	uint16	mode;
}
PSHeader;

/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/

static short unpack (uint8 *buffer, MEMFILE *mf, long width, long lines, long mod)
{
	char	 i;
	short	 j;
	long	 bytecount;
	long	 d;
	long	 count;
	long	 line;

	for (line = 0; line < lines; line++)
	{
		bytecount = 0;
		while (bytecount < width)
		{
		// read a char & check for eof

			d = MEMFILE_getc(mf);
			if (d == EOF)
			{
				return(1);
			}
			i = (char)d;

			if(i < 0)
			{
			// run data

				count = -i + 1;

				d = MEMFILE_getc(mf);
				if(d == EOF)
				{
					return(1);
				}

				for(j = 0; j < count; j++)
				{
					*buffer = (uint8)d;
					buffer += mod;
				}
			}
			else
			{
			// dump data

				count = i + 1;

				for (j = 0; j < count; j++)
				{
					d = MEMFILE_getc(mf);
					if(d == EOF)
					{
						return(1);
					}
					*buffer = (uint8)d;
					buffer += mod;
				}
			}

			bytecount += count;
		}
	}
	return 0;
}
// unpack

/*********************************************************************
 *
 * loadPhotoshop32Bit
 *
 * SYNOPSIS
 *		void  loadPhotoshop32Bit (BlockO24BitPixels *blockPtr, char *fileName)
 *
 * PURPOSE
 *		Attempts to load the specified photoshop 2.5 file.
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
int loadPhotoshop32Bit (BlockO32BitPixels *blockPtr, MEMFILE *mf)
{
	uint8		 header[4+2+6+2+4+4+2+2];
	uint8		**channelBufs = NULL;
	short		 r;
	short		 compress;
	short		 i;
	short		 line;
	long		 width;
	long		 height;
	long		 channels;
	long		 dsize;
	PSHeader	 psh;

// read the header

	if (MEMFILE_Read(mf, &header, sizeof(header)) != sizeof(header))
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf ("loadPhotoshop:Bad Header");
		goto cleanup;
	}

	psh.signature = ((uint32)header[ 0] << 24) | ((uint32)header[ 1] << 16) | ((uint32)header[ 2] << 8) | ((uint32)header[ 3]);
	psh.version   = ((uint16)header[ 4] <<  8) | ((uint16)header[ 5]);
	psh.channels  = ((uint16)header[12] <<  8) | ((uint16)header[13]);
	psh.rows      = ((uint32)header[14] << 24) | ((uint32)header[15] << 16) | ((uint32)header[16] << 8) | ((uint32)header[17]);
	psh.columns   = ((uint32)header[18] << 24) | ((uint32)header[19] << 16) | ((uint32)header[20] << 8) | ((uint32)header[21]);
	psh.depth     = ((uint16)header[22] <<  8) | ((uint16)header[23]);
	psh.mode      = ((uint16)header[24] <<  8) | ((uint16)header[25]);

// Verify the format

	if (psh.signature != 0x38425053)
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf1 ("loadPhotoshop:Cannot identify signature (%08lx)", psh.signature);
		goto cleanup;
	}
	if (psh.version != 1)
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf1 ("loadPhotoshop:Bad version number (%d)", psh.version);
		goto cleanup;
	}
	if (psh.channels < 3)
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf1 ("loadPhotoshop:Wrong number of channels (%d)", psh.channels);
		goto cleanup;
	}
	if (psh.depth != 8)
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf1 ("loadPhotoshop:Wrong depth (%d)", psh.depth);
		goto cleanup;
	}
	if (psh.mode != 3)
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf1 ("loadPhotoshop:Wrong Mode (not RGB(%d))", psh.mode);
		goto cleanup;
	}

	if (MEMFILE_Read(mf, &dsize, 4) != 4)
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf ("Error reading mode data");
		goto cleanup;
	}
	BigLong2Native (dsize);
	MEMFILE_Seek(mf, dsize, SEEK_CUR);

	if (MEMFILE_Read(mf, &dsize,4) != 4)
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf ("Error reading resource data");
		goto cleanup;
	}
	BigLong2Native (dsize);
	MEMFILE_Seek(mf, dsize, SEEK_CUR);

	if (MEMFILE_Read(mf, &dsize, 4) != 4)
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf ("Error reading reserved data");
		goto cleanup;
	}
	BigLong2Native (dsize);
	MEMFILE_Seek(mf, dsize, SEEK_CUR);

// check compression

	if (MEMFILE_Read(mf, &compress, 2) != 2)
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf ("Error reading compression type");
		goto cleanup;
	}
	BigWord2Native (compress);

	width    = psh.columns;
	height   = psh.rows;
	channels = psh.channels - 3;

	blockPtr->width    = width;
	blockPtr->height   = height;
	blockPtr->channels = channels;

	{
		long	channelSize;
		
		channelBufs = (uint8 **)malloc ((channels + 3) * sizeof (uint8 *));
		if (!channelBufs)
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf ("Out of memory reading photoshop file");
			goto cleanup;
		}

		channelSize = width * height;

		blockPtr->rgba = (pixel32 *)malloc (channelSize * 4);
		if (!blockPtr->rgba)
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf ("Out of memory reading photoshop file");
			goto cleanup;
		}

		memset (blockPtr->rgba, 255, channelSize * 4);

		channelBufs[0] = &blockPtr->rgba[0].red;
		channelBufs[1] = &blockPtr->rgba[0].green;
		channelBufs[2] = &blockPtr->rgba[0].blue;

		if (channels)
		{
			channelBufs[3] = &blockPtr->rgba[0].alpha;
			channels = 1;
		}
	}

// uncompress the body

	if (!compress)
	{
	// load uncompressed

		long	channel;

		for (channel = 0; channel < channels + 3; channel++)
		{
			for(line = 0; line < height; line++)
			{
				for(i = 0; i < width; i++)
				{
					if (EOF == (r = MEMFILE_getc(mf)))
					{
						SetGlobalErr (ERR_GENERIC);
						GEcatf ("Error reading pic data");
						goto cleanup;
					}
					*channelBufs[channel] = (uint8)r;
					channelBufs[channel] += 4;
				}
			}
		}
	}
	else
	{
	// load compressed
		long	junk;
		long	channel;

		junk = psh.rows*psh.channels*2;
		MEMFILE_Seek (mf, junk, SEEK_CUR);	// Skip line size info

		for (channel = 0; channel < channels + 3; channel++)
		{
			if (unpack(channelBufs[channel], mf, width, height, 4))
			{
				SetGlobalErr (ERR_GENERIC);
				GEcatf ("Error reading pic data");
				goto cleanup;
			}
		}
	}

	free (channelBufs);

//	flipBuffer (blockPtr->rgba, blockPtr->width * 4, blockPtr->height);

	return TRUE;

cleanup:

	if (channelBufs)	free (channelBufs);
	if (blockPtr->rgba)	free (blockPtr->rgba);

	return FALSE;
}
// loadPhotoshop32Bit


