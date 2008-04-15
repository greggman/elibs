/*************************************************************************
 *                                                                       *
 *                              READPCX.CPP                              *
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


   PROGRAMMERS


   FUNCTIONS

   TABS : 5 9

   HISTORY
		03/28/96 : Created.

 *************************************************************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include "echidna/readgfx.h"
#include "echidna/memfile.h"
#include "echidna/checkglu.h"
#include "echidna/eerrors.h"
#include "readpcx.h"

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/

typedef struct {
	uint8	Manufacturer;
	uint8	Version;
	uint8	Encoding;
	uint8	BitsPerPixel;
	uint16	XMin;
	uint16	YMin;
	uint16	XMax;
	uint16	YMax;
	uint16	HDPI;
	uint16	VDPI;
	uint8	Colormap[48];
	uint8	reserved0;
	uint8	NPlanes;
	uint16	BytesPerLine;
	uint16	PaletteInfo;
	uint16	HScreenSize;
	uint16	VScreenSize;
	uint8	reserved[54];
} PCXHeader;

/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/

/*********************************************************************
 *
 * Read256ColorPCX
 *
 * SYNOPSIS
 *		short Read256ColorPCX (IFFTracker *iff, ILBMInfo *ilbm)
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
int Read256ColorPCX (BlockO8BitPixels *bop, MEMFILE *mf, short bytesPerLine)
{
	long			 nRows = bop->height;
	long			 unpRowBytes = bop->width;
	long			 srcRowBytes = bytesPerLine;
	long			 dstRowBytes;
	long			 iRow;
	uint8			*buffer = 0;
	uint8			*pixels = 0;
	uint8			*pdest;
	int				 result = FALSE;

	dstRowBytes = bop->width;

	pixels = (uint8 *)malloc (bop->width * bop->height);
	buffer = (uint8 *)malloc (srcRowBytes);
	pdest  = pixels;

	if (!buffer || !pixels)
	{
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEcatf ("Out of memory reading pcx");
		goto RIBcleanup;
	}

	for (iRow = 0; iRow < nRows; iRow++)
	{
		uint8	*dst;
		uint8	 data;
		long	 len = srcRowBytes;

		dst = buffer;
		while (len > 0)
		{
			if ((data = MEMFILE_getc(mf)) == EOF)
			{
				SetGlobalErr (ERR_GENERIC);
				GEcatf ("Read256ColorPCX:Error reading file");
				goto RIBcleanup;
			}
			if ((data & 0xC0) == 0xC0)
			{
				uint8	count;

				count = data & 0x3F;
				if ((data = MEMFILE_getc(mf)) == EOF)
				{
					SetGlobalErr (ERR_GENERIC);
					GEcatf ("Read256ColorPCX:Error reading file");
					goto RIBcleanup;
				}
				memset (dst, data, count);
				len -= count;
				dst += count;
			} else {
				*dst++ = data;
				len--;
			}
		}

		memcpy (pdest, buffer, unpRowBytes);
		pdest += unpRowBytes;
	}

	{
		uint8	haspalette;

		if (MEMFILE_Seek (mf, -769L, SEEK_END) == -1)
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf ("Read256ColorPCX:Error reading palette (2)");
			goto RIBcleanup;
		}

		if (MEMFILE_Read (mf, &haspalette, 1) != 1)
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf ("Read256ColorPCX:No palette");
			goto RIBcleanup;
		}

		if (haspalette != 12)
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf ("Read256ColorPCX:Error reading palette");
			goto RIBcleanup;
		}

		if (MEMFILE_Read (mf, bop->palette, 768) != 768)
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf ("Read256ColorPCX:Error reading palette (3)");
			goto RIBcleanup;
		}
	}

	bop->pixels = pixels;

	result = TRUE;

RIBcleanup:
	if (buffer)				free (buffer);
	if (!result && pixels)	free (pixels);

	return result;
} /* Read256ColorPCX */

/*********************************************************************
 *
 * ReadPCX
 *
 * SYNOPSIS
 *		short ReadPCX (IFFTracker *iff)
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
int loadPCX (BlockO8BitPixels *bop, MEMFILE *mf)
{
	PCXHeader	 phead;

	if (MEMFILE_Read (mf, &phead, sizeof (phead)) != sizeof (phead))
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf ("ReadPCX:Couldn't read header info");
		goto cleanup;
	}
	
	LilWord2Native (phead.XMin);
	LilWord2Native (phead.YMin);
	LilWord2Native (phead.XMax);
	LilWord2Native (phead.YMax);
	LilWord2Native (phead.HDPI);
	LilWord2Native (phead.VDPI);
	LilWord2Native (phead.BytesPerLine);
	LilWord2Native (phead.PaletteInfo);
	LilWord2Native (phead.HScreenSize);
	LilWord2Native (phead.VScreenSize);

	if (phead.Encoding != 1)
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf1 ("ReadPCX:Can't read PCX files with type %d encoding (yet)", phead.Encoding);
		goto cleanup;
	}

	if (phead.BitsPerPixel != 8 || phead.NPlanes != 1 || phead.Version < 5)
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf ("ReadPCX:Can only 256 color PCX files (currently)");
		goto cleanup;
	}

	bop->width    = phead.XMax - phead.XMin + 1;
	bop->height   = phead.YMax - phead.YMin + 1;

	if (!Read256ColorPCX (bop, mf, phead.BytesPerLine))
	{
		goto cleanup;
	}		

	return TRUE;

cleanup:
	if (bop->pixels)	free(bop->pixels);
	return FALSE;

} /* ReadPCX */

/*************************************************************************
                                 loadPCX
 *************************************************************************

   SYNOPSIS
		int loadPCX (BlockO32BitPixels *bop, MEMFILE *mf)

   PURPOSE


   INPUT
		bop :
		mf  :

   OUTPUT
		None

   EFFECTS
		None

   RETURNS


   SEE ALSO


   HISTORY
		07/15/96 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int loadPCX32Bit (BlockO32BitPixels *bop, MEMFILE *mf)
BEGINFUNC (loadPCX32Bit)
{
	BlockO8BitPixels	bop8;

	if (loadPCX (&bop8, mf))
	{
		bop->channels = 0;
		bop->width    = bop8.width;
		bop->height   = bop8.height;

		bop->rgba = (pixel32 *)malloc (bop->width * bop->height * 4);
		if (!bop->rgba)
		{
			SetGlobalErr (ERR_OUT_OF_MEMORY);
			GEcatf ("Out of memory reading pcx");
			RETURN FALSE;
		}
	
		{
			pixel32	*d;
			uint8	*s;
			long	 size;
	
			s = bop8.pixels;
			d = bop->rgba;
			size = bop->width * bop->height;
	
			while (size)
			{
				int		 p;
	
				p        = *s++;
				d->red   = bop8.palette[p].red;
				d->green = bop8.palette[p].green;
				d->blue  = bop8.palette[p].blue;
				d->alpha = 255;

				d++;
				size--;
			}
		}

		free (bop8.pixels);

		RETURN TRUE;
	}

	RETURN FALSE;

} ENDFUNC (loadPCX32Bit)

/*************************************************************************
                               loadPCX8Bit
 *************************************************************************

   SYNOPSIS
		int loadPCX8Bit (BlockO8BitPixels *bop, MEMFILE *mf)

   PURPOSE


   INPUT
		bop :
		mf  :

   OUTPUT
		None

   EFFECTS
		None

   RETURNS


   SEE ALSO


   HISTORY
		07/15/96 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int loadPCX8Bit (BlockO8BitPixels *bop, MEMFILE *mf)
BEGINFUNC (loadPCX8Bit)
{
	RETURN loadPCX (bop, mf);

} ENDFUNC (loadPCX8Bit)

/*************************************************************************
                               SavePCX8Bit
 *************************************************************************

   SYNOPSIS
		int SavePCX8Bit (int fh, BlockO8BitPixels *bop)

   PURPOSE


   INPUT
		fh  :
		bop :

   OUTPUT
		None

   EFFECTS
		None

   RETURNS


   SEE ALSO


   TODO
 	* Shouldn't use CHK functions here because they exit


   HISTORY
		07/15/96 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int SavePCX8Bit (int fh, BlockO8BitPixels *bop)
BEGINFUNC (SavePCX8Bit)
{
	static PCXHeader pcx = 
	{
		10,					// UBYTE	Manufacturer;
		5,					// UBYTE	Version;
		1,					// UBYTE	Encoding;
		8,					// UBYTE	BitsPerPixel;
		0,					// short	XMin;
		0,					// short	YMin;
		0,					// short	XMax;
		0,					// short	YMax;
		LilWord(320),		// short	HDPI;
		LilWord(200),		// short	VDPI;
		{ 0, },				// UBYTE	Colormap[48];
		0,					// UBYTE	reserved0;
		1,					// UBYTE	NPlanes;
		0,					// short	BytesPerLine;
		0,					// short	PaletteInfo;
		LilWord(0),			// short	HScreenSize;
		LilWord(0),			// short	VScreenSize; 
	};
	
	long	 x;
	long	 y;
	long	 bytesPerLine;
	uint8	*linebuf;
	uint8	 value;
	uint8	*picture = bop->pixels;

	bytesPerLine = ((bop->width + 1) & 0x7FFFFFFE);

	pcx.XMax = (short)(bop->width  - 1);
	pcx.YMax = (short)(bop->height - 1);
	pcx.BytesPerLine = (short)bytesPerLine;

	linebuf = CHK_CallocateMemory (bytesPerLine * 3, "linebuf");

	MakeLilWord (pcx.XMax);
	MakeLilWord (pcx.YMax);
	MakeLilWord (pcx.BytesPerLine);

	memcpy (&pcx.Colormap[0], bop->palette, 48);

	CHK_Write (fh, &pcx, sizeof (pcx));

	for (y = 0; y < bop->height; y++)
	{
		uint8	*dst;
		long	 cnt;

		cnt = 0;
		dst = linebuf;

		for (x = 0; x < bop->width; x++)
		{
			if (*picture >= 0xC0)
			{
				*dst++ = 0xC1;
				cnt++;
			}
			*dst++ = *picture++;
			cnt++;
		}

		if (bop->width & 0x01)
		{
			*dst++ = 0xC1;
			*dst++ = 0xFF;
			cnt += 2;
		}

		CHK_Write (fh, linebuf, cnt);
	}

	value = 0x0C;
	CHK_Write (fh, &value, 1);
	CHK_Write (fh, bop->palette, 768);

	CHK_DeallocateMemory (linebuf, "linebuf");

	RETURN TRUE;

} ENDFUNC (SavePCX8Bit)

