/*************************************************************************
 *                                                                       *
 *                              READTGA.CPP                              *
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

/**************************** I N C L U D E S ****************************/
#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include "echidna/readgfx.h"
#include "echidna/checkglu.h"
#include "echidna/eerrors.h"
#include "readtga.h"

/**************************** C O N S T A N T S ***************************/

// descriptor: 00vhaaaa
// h horizontal flip
// v vertical flip
// a alpha bits
#define TGA_IDESC_HFLIP 0x10
#define TGA_IDESC_VFLIP 0x20

/******************************** T Y P E S *******************************/

typedef struct TGAHeader
{
    uint8   ID;             // Byte 0	num bytes in ID field
    uint8   ctype;          // Byte 1	0 = no colormap, 1 = colormap
    uint8   itype;          // Byte 2	0 = none,
							//			1 = uncompressed colormap img
							//			2 = uncompressed truecolor
							//			3 = uncompressed black&white
							//			9 = rle colormapped
							//			10 = rle truecolor
							//			11 = rle black and white
    uint8   mincolorl;      // Byte 3
    uint8   mincolorh;      // Byte 4
    uint8   colorsl;        // Byte 5
    uint8   colorsh;        // Byte 6
    uint8   colorsize;      // Byte 7
    uint8   originxl;       // Byte 8
    uint8   originxh;       // Byte 9
    uint8   originyl;       // Byte 10
    uint8   originyh;       // Byte 11
    uint8   widthl;         // Byte 12
    uint8   widthh;         // Byte 13
    uint8   heightl;        // Byte 14
    uint8   heighth;        // Byte 15
    uint8   bpp;            // Byte 16
    uint8   idesc;          // Byte 17
} TGAHeader;

/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/

/*********************************************************************
 *
 * loadUncompressedTGA
 *
 * SYNOPSIS
 *      void  loadUncompressedTGA (BlockO32BitPixels *blockPtr, unsigned char *fileBufferPtr)
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
static int loadUncompressedTGA (BlockO32BitPixels *blockPtr, int channels, MEMFILE *mf, pixel32* colorMap, uint8 idesc)
{

	pixel32* bufferStart;
	pixel32* buffer;

	long	 i;
	long	 loop;
	long	 lineSize;
	long	 bufferSize;
	long	 bufferWidth;
	long	 bufferHeight;
	long	 lineMod;

	bufferWidth  = blockPtr->width;
	bufferHeight = blockPtr->height;
	lineSize     = bufferWidth;
	bufferSize   = lineSize * bufferHeight;

	#if READ_UPSIDEDOWN
		if (idesc & TGA_IDESC_VFLIP)
		{
			bufferStart  = blockPtr->rgba + bufferSize - lineSize;
			lineMod      = -lineSize;
		}
		else
		{
			bufferStart  = blockPtr->rgba;
			lineMod      = lineSize;
		}
    #else
		if (idesc & TGA_IDESC_VFLIP)
		{
			bufferStart  = blockPtr->rgba;
			lineMod      = lineSize;
		}
		else
		{
			bufferStart  = blockPtr->rgba + bufferSize - lineSize;
			lineMod      = -lineSize;
		}
	#endif

	buffer = bufferStart;

	switch (channels)
	{
	case 1:
		if (colorMap)
		{
			// No Alpha / Uncompressed

			for (loop = 0; loop < bufferHeight; loop++)
			{
				buffer = bufferStart;

				for (i = 0; i < bufferWidth; i++)
				{
					uint8	k;

					k = MEMFILE_getc(mf);

					buffer->red   = colorMap[k].red;
					buffer->green = colorMap[k].green;
					buffer->blue  = colorMap[k].blue;
					buffer->alpha = colorMap[k].alpha;
					buffer++;
				}

				bufferStart += lineMod;
			}
		}
		else
		{
			// No Alpha / Uncompressed

			for (loop = 0; loop < bufferHeight; loop++)
			{
				buffer = bufferStart;

				for (i = 0; i < bufferWidth; i++)
				{
					uint8	k;

					k = MEMFILE_getc(mf);

					buffer->red   = k;
					buffer->green = k;
					buffer->blue  = k;
					buffer->alpha = 255;
					buffer++;
				}

				bufferStart += lineMod;
			}
		}
		break;
	case 3:
		{
			// No Alpha / Uncompressed

			for (loop = 0; loop < bufferHeight; loop++)
			{
				buffer = bufferStart;

				for (i = 0; i < bufferWidth; i++)
				{
					uint8	r,g,b;

					b = MEMFILE_getc(mf);
					g = MEMFILE_getc(mf);
					r = MEMFILE_getc(mf);

					buffer->red   = r;
					buffer->green = g;
					buffer->blue  = b;
					buffer->alpha = 255;
					buffer++;
				}

				bufferStart += lineMod;
			}
		}
		break;
	case 4:
		{
			// Alpha / Uncompressed

			for (loop = 0; loop < bufferHeight; loop++)
			{
				buffer = bufferStart;

				for (i = 0; i < bufferWidth; i++)
				{
					uint8	r,g,b,a;

					b = MEMFILE_getc(mf);
					g = MEMFILE_getc(mf);
					r = MEMFILE_getc(mf);
					a = MEMFILE_getc(mf);

					buffer->red   = r;
					buffer->green = g;
					buffer->blue  = b;
					buffer->alpha = a;
					buffer++;
				}
				bufferStart += lineMod;
			}
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
// loadUncompressedTGA

/*********************************************************************
 *
 * loadUncompressedGrey8BitTGA
 *
 * SYNOPSIS
 *      void  loadUncompressedGrey8BitTGA (BlockOGrey8BitPixels *blockPtr, unsigned char *fileBufferPtr)
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
static int loadUncompressedGrey8BitTGA (BlockOGrey8BitPixels *blockPtr, MEMFILE *mf, uint8 idesc)
{

    uint8* bufferStart;
    uint8* buffer;

    long     i;
    long     loop;
    long     lineSize;
    long     bufferSize;
    long     bufferWidth;
    long     bufferHeight;

    bufferWidth  = blockPtr->width;
    bufferHeight = blockPtr->height;
    lineSize     = bufferWidth;
    bufferSize   = lineSize * bufferHeight;

    bufferStart  = blockPtr->pixels + bufferSize - lineSize;

    buffer = bufferStart;

    for (loop = 0; loop < bufferHeight; loop++)
    {
        buffer = bufferStart;

        for (i = 0; i < bufferWidth; i++)
        {
            uint8   a;

            a = MEMFILE_getc(mf);

            *buffer = a;
            buffer++;
        }

        bufferStart -= lineSize;
    }

    if (idesc & TGA_IDESC_VFLIP)
    {
        return flipBuffer (blockPtr->pixels, blockPtr->width, blockPtr->height);
    }
    else
    {
        return TRUE;
    }
}
// loadUncompressedGrey8BitTGA

/*********************************************************************
 *
 * loadCompressedGrey8BitTGA
 *
 * SYNOPSIS
 *      void  loadCompressedGrey8BitTGA (BlockOGrey8BitPixels *blockPtr, uint8 *fileBufferPtr)
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
int loadCompressedGrey8BitTGA (BlockOGrey8BitPixels *blockPtr, MEMFILE *mf, uint8 idesc)
{
    uint8           a;
    uint8           i;
    uint8           count;
    uint8*          buffer;

    long            pixelCount;
    long            totalPixels;
    long            bufferWidth;
    long            bufferHeight;

    bufferWidth  = blockPtr->width;
    bufferHeight = blockPtr->height;
    totalPixels  = bufferWidth * bufferHeight;
    buffer       = blockPtr->pixels;
    pixelCount   = 0;

    while (pixelCount < totalPixels)
    {
        i = MEMFILE_getc(mf);
        count = (0x7F & i) + 1;

        pixelCount += count;

        if (pixelCount > totalPixels)
        {
            SetGlobalErr (ERR_GENERIC);
            GEcatf ("Pixel overflow in targa decompression");
            return FALSE;
        }

        if (i & 0x80)
        {
            // run data

            a = MEMFILE_getc(mf);

            while (count--)
            {
                *buffer = a;
                buffer++;
            }
        }
        else
        {
            // dump data

            while (count--)
            {
                a = MEMFILE_getc(mf);

                *buffer = a;
                buffer++;
            }
        }
    }

    if (idesc & TGA_IDESC_VFLIP)
    {
        return TRUE;
    }
    else
    {
		return flipBuffer (blockPtr->pixels, blockPtr->width, blockPtr->height);
    }
}
// loadCompressedGrey8BitTGA

/*********************************************************************
 *
 * loadTGAGrey8Bit
 *
 * SYNOPSIS
 *      void loadTGAGrey8Bit (BlockOGrey8BitPixels *blockPtr, char *fileName)
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
int loadTGAGrey8Bit (BlockOGrey8BitPixels *blockPtr, MEMFILE *mf)
{
    TGAHeader        tgaHeaderX;
    TGAHeader       *tgaHeader = &tgaHeaderX;

    long            bufferWidth;
    long            bufferHeight;

    MEMFILE_Read(mf, tgaHeader, sizeof (TGAHeader));

    MEMFILE_Seek (mf, tgaHeader->ID, SEEK_CUR); // Skip User Info

    bufferWidth  = ((long)tgaHeader->widthl  + (long)tgaHeader->widthh * 256L);
    bufferHeight = ((long)tgaHeader->heightl + (long)tgaHeader->heighth * 256L);

    {
        long    bufferSize;

        bufferSize = bufferWidth * bufferHeight;

        blockPtr->pixels = (uint8 *) malloc (bufferSize);
        blockPtr->width  = bufferWidth;
        blockPtr->height = bufferHeight;

        if (!blockPtr->pixels)
        {
            SetGlobalErr (ERR_GENERIC);
            GEcatf ("Out of Memory loading tga");
            goto cleanup;
        }
    }

    switch (tgaHeader->itype)
    {
    case 2:
        if (tgaHeader->bpp != 8)
        {
            SetGlobalErr (ERR_GENERIC);
            GEcatf1 ("Unsupported bit depth (%d)", tgaHeader->bpp);
            goto cleanup;
        }
        else
        {
// printf ("load uncompressed tga\n");
            if (!loadUncompressedGrey8BitTGA (blockPtr, mf, tgaHeader->idesc))
            {
                goto cleanup;
            }
        }
        break;

    case 10:
        if (tgaHeader->bpp != 8)
        {
            SetGlobalErr (ERR_GENERIC);
            GEcatf1 ("Unsupported bit depth (%d)", tgaHeader->bpp);
            goto cleanup;
        }
        else
        {
// printf ("load compressed tga\n");
            if (!loadCompressedGrey8BitTGA (blockPtr, mf, tgaHeader->idesc))
            {
                goto cleanup;
            }
        }
        break;
    default:
        SetGlobalErr (ERR_GENERIC);
        GEcatf ("Unsupported TGA file type");
        goto cleanup;
    }

    return TRUE;

cleanup:
    if (blockPtr->pixels)
    {
        free (blockPtr->pixels);
    }
    return FALSE;
}
// loadTGAGrey8Bit

/*********************************************************************
 *
 * flipBuffer
 *
 * SYNOPSIS
 *      void  flipBuffer (void *buffer, long rowSize, long rows)
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
int flipBuffer (void *buffer, long rowSize, long rows)
{

    uint8   *temp;
    uint8   *firstRow;
    uint8   *lastRow;

    long     count;

    temp = (uint8 *)malloc (rowSize);
    if (!temp)
    {
        SetGlobalErr (ERR_GENERIC);
        GEcatf ("Out of memory loading tga");
        return FALSE;
    }

    firstRow = (uint8 *) buffer;
    lastRow  = firstRow + rowSize * (rows - 1);

    count = (rows + 1) / 2;

    while (count)
    {
        memcpy (temp, lastRow, rowSize);
        memcpy (lastRow, firstRow, rowSize);
        memcpy (firstRow, temp, rowSize);

        firstRow += rowSize;
        lastRow  -= rowSize;

        count--;
    }

    free (temp);

    return TRUE;
}
// flipBuffer

/*********************************************************************
 *
 * loadCompressedTGA
 *
 * SYNOPSIS
 *		void  loadCompressedTGA (BlockO32BitPixels *blockPtr, uint8 *fileBufferPtr)
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
int loadCompressedTGA (BlockO32BitPixels *blockPtr, int channels, MEMFILE *mf, pixel32* colorMap, uint8 idesc)
{
	uint8			a;
	uint8			r;
	uint8			g;
	uint8			b;
	uint8			i;
	uint8			count;
	pixel32*		buffer;

	long			pixelCount;
	long			totalPixels;
	long			bufferWidth;
	long			bufferHeight;

	bufferWidth  = blockPtr->width;
	bufferHeight = blockPtr->height;
	totalPixels  = bufferWidth * bufferHeight;
	buffer       = blockPtr->rgba;
	pixelCount   = 0;

	while (pixelCount < totalPixels)
	{
		i = MEMFILE_getc(mf);
		count = (0x7F & i) + 1;

		pixelCount += count;

		if (pixelCount > totalPixels)
		{
			SetGlobalErr(ERR_GENERIC);
			GEcatf ("Pixel overflow in targa decompression\n");
			return FALSE;
		}

		if (i & 0x80)
		{
			// run data
			switch (channels)
			{
			case 1:
				if (colorMap)
				{
					uint8 k;
					k = MEMFILE_getc(mf);

					r = colorMap[k].red;
					g = colorMap[k].green;
					b = colorMap[k].blue;
					a = colorMap[k].alpha;
				}
				else
				{
					b = MEMFILE_getc(mf);
					g = b;
					r = b;
					a = 255;
				}
				break;
			case 3:
				b = MEMFILE_getc(mf);
				g = MEMFILE_getc(mf);
				r = MEMFILE_getc(mf);
				a = 255;
				break;
			case 4:
				b = MEMFILE_getc(mf);
				g = MEMFILE_getc(mf);
				r = MEMFILE_getc(mf);
				a = MEMFILE_getc(mf);
				break;
			}

			while (count--)
			{
				buffer->red   = r;
				buffer->green = g;
				buffer->blue  = b;
				buffer->alpha = a;
				buffer++;
			}
		}
		else
		{
			// dump data

			while (count--)
			{
				switch (channels)
				{
				case 1:
					if (colorMap)
					{
						uint8 k;
						k = MEMFILE_getc(mf);

						r = colorMap[k].red;
						g = colorMap[k].green;
						b = colorMap[k].blue;
						a = colorMap[k].alpha;
					}
					else
					{
						b = MEMFILE_getc(mf);
						g = b;
						r = b;
						a = 255;
					}
					break;
				case 3:
					b = MEMFILE_getc(mf);
					g = MEMFILE_getc(mf);
					r = MEMFILE_getc(mf);
					break;
				case 4:
					b = MEMFILE_getc(mf);
					g = MEMFILE_getc(mf);
					r = MEMFILE_getc(mf);
					a = MEMFILE_getc(mf);
					break;
				}

				buffer->red   = r;
				buffer->green = g;
				buffer->blue  = b;
				buffer->alpha = a;
				buffer++;
			}
		}
	}

    #if READ_UPSIDEDOWN
		if (idesc & TGA_IDESC_VFLIP)
		{
			flipBuffer (blockPtr->rgba, blockPtr->width * sizeof (pixel32), blockPtr->height);
		}
		else
		{
		}
    #else
		if (idesc & TGA_IDESC_VFLIP)
		{
		}
		else
		{
			flipBuffer (blockPtr->rgba, blockPtr->width * sizeof (pixel32), blockPtr->height);
		}
	#endif

	return TRUE;
}
// loadCompressedTGA


/*********************************************************************
 *
 * loadTGA32Bit
 *
 * SYNOPSIS
 *      void loadTGA32Bit (BlockO32BitPixels *blockPtr, char *fileName)
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
#define MAX_COLORMAP 256

int loadTGA32Bit (BlockO32BitPixels *blockPtr, MEMFILE *mf)
{
    TGAHeader        tgaHeaderX;
    TGAHeader       *tgaHeader = &tgaHeaderX;

    long            alphachannels;
    long            bufferWidth;
    long            bufferHeight;

	pixel32			colorMap[MAX_COLORMAP];
	int				fColorMap = FALSE;
	int				channels = 0;

    MEMFILE_Read(mf, tgaHeader, sizeof (TGAHeader));

    #if 0
    {
        printf ("\n");
        printf ("\t\ttgaHeader->ID          = %d\n", tgaHeader->ID);
        printf ("\t\ttgaHeader->ctype       = %d\n", tgaHeader->ctype);
        printf ("\t\ttgaHeader->itype       = %d\n", tgaHeader->itype);
        printf ("\t\ttgaHeader->mincolorl   = %d\n", tgaHeader->mincolorl);
        printf ("\t\ttgaHeader->mincolorh   = %d\n", tgaHeader->mincolorh);
        printf ("\t\ttgaHeader->colorsl     = %d\n", tgaHeader->colorsl);
        printf ("\t\ttgaHeader->colorsh     = %d\n", tgaHeader->colorsh);
        printf ("\t\ttgaHeader->colorsize   = %d\n", tgaHeader->colorsize);
        printf ("\t\ttgaHeader->originxl    = %d\n", tgaHeader->originxl);
        printf ("\t\ttgaHeader->originxh    = %d\n", tgaHeader->originxh);
        printf ("\t\ttgaHeader->originyl    = %d\n", tgaHeader->originyl);
        printf ("\t\ttgaHeader->originyh    = %d\n", tgaHeader->originyh);
        printf ("\t\ttgaHeader->widthl      = %d\n", tgaHeader->widthl);
        printf ("\t\ttgaHeader->widthh      = %d\n", tgaHeader->widthh);
        printf ("\t\ttgaHeader->heightl     = %d\n", tgaHeader->heightl);
        printf ("\t\ttgaHeader->heighth     = %d\n", tgaHeader->heighth);
        printf ("\t\ttgaHeader->bpp         = %d\n", tgaHeader->bpp);
        printf ("\t\ttgaHeader->idesc       = %d\n", tgaHeader->idesc);
        printf ("\n");
    }
    #endif

    MEMFILE_Seek (mf, tgaHeader->ID, SEEK_CUR); // Skip User Info

    alphachannels = FALSE;
    if (tgaHeader->bpp == 32)
    {
        alphachannels = TRUE;
		channels = 4;
    }
	else if (tgaHeader->bpp == 24)
	{
		channels = 3;
	}
	else if (tgaHeader->bpp == 8)
	{
		int	firstColor = ((long)tgaHeader->mincolorl + (long)tgaHeader->mincolorh * 256L);
		int	numColors  = ((long)tgaHeader->colorsl   + (long)tgaHeader->colorsh   * 256L);

		channels = 1;

		fColorMap = TRUE;

		if (numColors > 0 && firstColor + numColors <= 256)
		{
			int colorNdx;
			memset (colorMap, 0, sizeof (colorMap));

			switch (tgaHeader->colorsize)
			{
			case 24:
				for (colorNdx = 0; colorNdx < numColors; ++colorNdx)
				{
					uint8 r,g,b;

					b = MEMFILE_getc(mf);
					g = MEMFILE_getc(mf);
					r = MEMFILE_getc(mf);

					colorMap[firstColor + colorNdx].red   = r;
					colorMap[firstColor + colorNdx].green = g;
					colorMap[firstColor + colorNdx].blue  = b;
					colorMap[firstColor + colorNdx].alpha = 255;
				}
				break;
			case 32:
				for (colorNdx = 0; colorNdx < numColors; ++colorNdx)
				{
					uint8 r,g,b,a;
					b = MEMFILE_getc(mf);
					g = MEMFILE_getc(mf);
					r = MEMFILE_getc(mf);
					a = MEMFILE_getc(mf);

					colorMap[firstColor + colorNdx].red   = r;
					colorMap[firstColor + colorNdx].green = g;
					colorMap[firstColor + colorNdx].blue  = b;
					colorMap[firstColor + colorNdx].alpha = a;
				}
				break;
			default:
				SetGlobalErr (ERR_GENERIC);
				GEcatf ("unhandled color map size\n");
				goto cleanup;
			}
		}
		else if (numColors)
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf ("too many colors in color map\n");
			goto cleanup;
		}
	}
	else
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf ("unsupported bits per pixel\n");
		goto cleanup;
	}

    bufferWidth  = ((long)tgaHeader->widthl  + (long)tgaHeader->widthh * 256L);
    bufferHeight = ((long)tgaHeader->heightl + (long)tgaHeader->heighth * 256L);

    {
        long    bufferSize;

        bufferSize = bufferWidth * bufferHeight * 4;

        blockPtr->rgba   = (pixel32 *) malloc (bufferSize);
        blockPtr->width  = bufferWidth;
        blockPtr->height = bufferHeight;

        if (alphachannels)
        {
            blockPtr->channels   = 1;
        }

        if (!blockPtr->rgba)
        {
            SetGlobalErr (ERR_GENERIC);
            GEcatf ("Out of Memory loading tga");
            goto cleanup;
        }
    }

    switch (tgaHeader->itype)
    {
	case 1: // uncompressed color map
		if (tgaHeader->bpp != 8)
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("Unsupported bit depth (%d)\n", tgaHeader->bpp);
			goto cleanup;
		}
		else
		{
			if (!loadUncompressedTGA (blockPtr, channels, mf, colorMap, tgaHeader->idesc))
			{
				goto cleanup;
			}
		}
		break;
	case 2: // uncompressed true color
		if (((tgaHeader->bpp != 24) && (tgaHeader->bpp != 32)))
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("Unsupported bit depth (%d)\n", tgaHeader->bpp);
			goto cleanup;
		}
		else
		{
			if (!loadUncompressedTGA (blockPtr, channels, mf, NULL, tgaHeader->idesc))
			{
				goto cleanup;
			}
		}
		break;
	case 3: // uncompressed black & white
		if (tgaHeader->bpp != 8)
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1("Unsupported bit depth (%d)\n", tgaHeader->bpp);
			goto cleanup;
		}
		else
		{
			if (!loadUncompressedTGA (blockPtr, channels, mf, NULL, tgaHeader->idesc))
			{
				goto cleanup;
			}
		}
		break;
	case 9: // rle compressed color map
		if (tgaHeader->bpp != 8)
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("Unsupported bit depth (%d)\n", tgaHeader->bpp);
			goto cleanup;
		}
		else
		{
			if (!loadCompressedTGA (blockPtr, channels, mf, colorMap, tgaHeader->idesc))
			{
				goto cleanup;
			}
		}
		break;
	case 10: // rle compressed black & white
		if (((tgaHeader->bpp != 24) && (tgaHeader->bpp != 32)))
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("Unsupported bit depth (%d)\n", tgaHeader->bpp);
			goto cleanup;
		}
		else
		{
			if (!loadCompressedTGA (blockPtr, channels, mf, NULL, tgaHeader->idesc))
			{
				goto cleanup;
			}
		}
		break;
	case 11: // rle compressed black & white
		if (tgaHeader->bpp != 8)
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("Unsupported bit depth (%d)\n", tgaHeader->bpp);
			goto cleanup;
		}
		else
		{
			if (!loadCompressedTGA (blockPtr, channels, mf, NULL, tgaHeader->idesc))
			{
				goto cleanup;
			}
		}
		break;
	default:
        SetGlobalErr (ERR_GENERIC);
        GEcatf ("Unsupported TGA file type\n");
        goto cleanup;
    }

    return TRUE;

cleanup:
    if (blockPtr->rgba)
    {
        free (blockPtr->rgba);
    }
    return FALSE;
}

// loadTGA32Bit

/*************************************************************************
                              saveTGA32Bit
 *************************************************************************

   SYNOPSIS
        int saveTGA32Bit (int fh, BlockO32BitPixels *bop)

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
    * Don't use CHK functions cause they exit


   HISTORY
        07/15/96 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int saveTGA32Bit (int fh, BlockO32BitPixels *bop)
BEGINFUNC (saveTGA32Bit)
{
    static TGAHeader tgaHeader =
    {
    0,  //  uint8   ID;             // Byte 0
    0,  //  uint8   ctype;          // Byte 1
    2,  //  uint8   itype;          // Byte 2
    0,  //  uint8   mincolorl;      // Byte 3
    0,  //  uint8   mincolorh;      // Byte 4
    0,  //  uint8   colorsl;        // Byte 5
    0,  //  uint8   colorsh;        // Byte 6
    0,  //  uint8   colorsize;      // Byte 7
    0,  //  uint8   originxl;       // Byte 8
    0,  //  uint8   originxh;       // Byte 9
    0,  //  uint8   originyl;       // Byte 10
    0,  //  uint8   originyh;       // Byte 11
    0,  //  uint8   widthl;         // Byte 12
    0,  //  uint8   widthh;         // Byte 13
    0,  //  uint8   heightl;        // Byte 14
    0,  //  uint8   heighth;        // Byte 15
    32, //  uint8   bpp;            // Byte 16
    0,  //  uint8   idesc;          // Byte 17
    };

    uint8 *BRGABuffer;

    BRGABuffer = (uint8 *)malloc (bop->width * bop->height * 4);
    if (!BRGABuffer)
    {
        SetGlobalErr (ERR_GENERIC);
        GEcatf ("Out of memory saving tga");
        return FALSE;
    }

    tgaHeader.widthl = (UINT8)(bop->width % 256);
    tgaHeader.widthh = (UINT8)(bop->width / 256);

    tgaHeader.heightl = (UINT8)(bop->height % 256);
    tgaHeader.heighth = (UINT8)(bop->height / 256);

    CHK_Write (fh, &tgaHeader, sizeof (tgaHeader));

    {
        pixel32 *s = bop->rgba;
        uint8   *d = BRGABuffer;
        long     count = bop->width * bop->height;

        while (count)
        {
            *d++ = s->blue;
            *d++ = s->green;
            *d++ = s->red;
            *d++ = s->alpha;

            s++;
            count--;
        }
    }

    flipBuffer (BRGABuffer, bop->width * 4, bop->height);
    CHK_Write (fh, BRGABuffer, bop->width * bop->height * 4);
    free (BRGABuffer);

    return TRUE;

} ENDFUNC (saveTGA32Bit)

/*************************************************************************
                               saveTGAGrey8Bit
 *************************************************************************

   SYNOPSIS
        int saveTGAGrey8Bit (int fh, BlockOGrey8BitPixels *bop)

   PURPOSE
        Saves an Grey8Bit targa

   INPUT
        fh  :
        bop :

   OUTPUT
        None

   EFFECTS
        None

   RETURNS


   SEE ALSO


   HISTORY
        07/15/04 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int saveTGAGrey8Bit (int fh, BlockOGrey8BitPixels *bop)
BEGINFUNC (saveTGAGrey8Bit)
{
    static TGAHeader tgaHeader =
    {
    0,  //  uint8   ID;             // Byte 0
    0,  //  uint8   ctype;          // Byte 1
    3,  //  uint8   itype;          // Byte 2 -- uncompressed 8bit greyscale
    0,  //  uint8   mincolorl;      // Byte 3
    0,  //  uint8   mincolorh;      // Byte 4
    0,  //  uint8   colorsl;        // Byte 5
    0,  //  uint8   colorsh;        // Byte 6
    0,  //  uint8   colorsize;      // Byte 7
    0,  //  uint8   originxl;       // Byte 8
    0,  //  uint8   originxh;       // Byte 9
    0,  //  uint8   originyl;       // Byte 10
    0,  //  uint8   originyh;       // Byte 11
    0,  //  uint8   widthl;         // Byte 12
    0,  //  uint8   widthh;         // Byte 13
    0,  //  uint8   heightl;        // Byte 14
    0,  //  uint8   heighth;        // Byte 15
    8,  //  uint8   bpp;            // Byte 16
    0,  //  uint8   idesc;          // Byte 17
    };

    uint8 *buffer;

    buffer = (uint8 *)malloc (bop->width * bop->height);
    if (!buffer)
    {
        SetGlobalErr (ERR_GENERIC);
        GEcatf ("Out of memory saving tga");
        return FALSE;
    }

    tgaHeader.widthl = (UINT8)(bop->width % 256);
    tgaHeader.widthh = (UINT8)(bop->width / 256);

    tgaHeader.heightl = (UINT8)(bop->height % 256);
    tgaHeader.heighth = (UINT8)(bop->height / 256);

    CHK_Write (fh, &tgaHeader, sizeof (tgaHeader));

    memcpy (buffer, bop->pixels, bop->width * bop->height);
    flipBuffer (buffer, bop->width * 1, bop->height);
    CHK_Write (fh, buffer, bop->width * bop->height * 1);
    free (buffer);

    RETURN TRUE;

} ENDFUNC (saveTGAGrey8Bit)


