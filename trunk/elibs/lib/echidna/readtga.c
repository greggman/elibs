/*************************************************************************
 *                                                                       *
 *                              READTGA.CPP                              *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

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


/******************************** T Y P E S *******************************/

typedef struct TGAHeader
{
	uint8	ID;				// Byte 0
	uint8	ctype;			// Byte 1
	uint8	itype;			// Byte 2
	uint8	mincolorl;		// Byte 3
	uint8	mincolorh;		// Byte 4
	uint8	colorsl;		// Byte 5
	uint8	colorsh;		// Byte 6
	uint8	colorsize;		// Byte 7
	uint8	originxl;		// Byte 8
	uint8	originxh;		// Byte 9
	uint8	originyl;		// Byte 10
	uint8	originyh;		// Byte 11
	uint8	widthl;			// Byte 12
	uint8	widthh;			// Byte 13
	uint8	heightl;		// Byte 14
	uint8	heighth;		// Byte 15
	uint8	bpp;			// Byte 16
	uint8	idesc;			// Byte 17
} TGAHeader;

/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/

/*********************************************************************
 *
 * loadUncompressedTGA
 *
 * SYNOPSIS
 *		void  loadUncompressedTGA (BlockO32BitPixels *blockPtr, unsigned char *fileBufferPtr)
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
static int loadUncompressedTGA (BlockO32BitPixels *blockPtr, MEMFILE *mf)
{

	pixel32* bufferStart;
	pixel32* buffer;

	long	 i;
	long	 loop;
	long	 lineSize;
	long	 bufferSize;
	long	 bufferWidth;
	long	 bufferHeight;

	bufferWidth  = blockPtr->width;
	bufferHeight = blockPtr->height;
	lineSize     = bufferWidth;
	bufferSize   = lineSize * bufferHeight;
	
	bufferStart  = blockPtr->rgba + bufferSize - lineSize;

	buffer = bufferStart;
	
	if (!blockPtr->channels)
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
			
			bufferStart -= lineSize;
		}
	}
	else
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
			bufferStart -= lineSize;
		}
	}
	return TRUE;
}
// loadUncompressedTGA

/*********************************************************************
 *
 * flipBuffer
 *
 * SYNOPSIS
 *		void  flipBuffer (void *buffer, long rowSize, long rows)
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

	uint8	*temp;
	uint8	*firstRow;
	uint8	*lastRow;
			
	long	 count;
	
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
int loadCompressedTGA (BlockO32BitPixels *blockPtr, MEMFILE *mf)
{
	uint8			a;
	uint8			r;
	uint8			g;
	uint8			b;
	uint8			i;
	uint8			count;
	pixel32*		buffer;

	int				hasAlpha;

	long			pixelCount;
	long			totalPixels;
	long			bufferWidth;
	long			bufferHeight;

	bufferWidth  = blockPtr->width;
	bufferHeight = blockPtr->height;
	totalPixels  = bufferWidth * bufferHeight;
	buffer       = blockPtr->rgba;
	pixelCount   = 0;
	hasAlpha     = blockPtr->channels;
	a            = 255;

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

			b = MEMFILE_getc(mf);
			g = MEMFILE_getc(mf);
			r = MEMFILE_getc(mf);

			if (hasAlpha)
			{
				a = MEMFILE_getc(mf);
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
				b = MEMFILE_getc(mf);
				g = MEMFILE_getc(mf);
				r = MEMFILE_getc(mf);
			
				if (hasAlpha)
				{
					a = MEMFILE_getc(mf);
				}

				buffer->red   = r;
				buffer->green = g;
				buffer->blue  = b;
				buffer->alpha = a;
				buffer++;
			}
		}
	}
	
	return flipBuffer (blockPtr->rgba, blockPtr->width * 4, blockPtr->height);
//	return TRUE;
}
// loadCompressedTGA


/*********************************************************************
 *
 * loadTGA32Bit
 *
 * SYNOPSIS
 *		void loadTGA32Bit (BlockO32BitPixels *blockPtr, char *fileName)
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
int loadTGA32Bit (BlockO32BitPixels *blockPtr, MEMFILE *mf)
{
	TGAHeader		 tgaHeaderX;
	TGAHeader		*tgaHeader = &tgaHeaderX;
	
	long			alphachannels;
	long			bufferWidth;
	long			bufferHeight;

	MEMFILE_Read(mf, tgaHeader, sizeof (TGAHeader));
	
	if (tgaHeader->idesc & 0xf0)
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf ("Invalid TGA file");
		goto cleanup;
	}

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
	
	if (tgaHeader->bpp == 32)
	{
		alphachannels = TRUE;
	}
	else
	{
		alphachannels = FALSE;
	}
		
	bufferWidth  = ((long)tgaHeader->widthl  + (long)tgaHeader->widthh * 256L);
	bufferHeight = ((long)tgaHeader->heightl + (long)tgaHeader->heighth * 256L);
	
	{
		long	bufferSize;
				
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
	case 2:
		if (((tgaHeader->bpp != 24) && (tgaHeader->bpp != 32)))
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("Unsupported bit depth (%d)", tgaHeader->bpp);
			goto cleanup;
		}
		else
		{
// printf ("load uncompressed tga\n");
			if (!loadUncompressedTGA (blockPtr, mf))
			{
				goto cleanup;
			}
		}
		break;
	
	case 10:
		if (((tgaHeader->bpp != 24) && (tgaHeader->bpp != 32)))
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("Unsupported bit depth (%d)", tgaHeader->bpp);
			goto cleanup;
		}
		else
		{
// printf ("load compressed tga\n");
			if (!loadCompressedTGA (blockPtr, mf))
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
	0,	//	uint8	ID;				// Byte 0
	0,	//	uint8	ctype;			// Byte 1
	2,	//	uint8	itype;			// Byte 2
	0,	//	uint8	mincolorl;		// Byte 3
	0,	//	uint8	mincolorh;		// Byte 4
	0,	//	uint8	colorsl;		// Byte 5
	0,	//	uint8	colorsh;		// Byte 6
	0,	//	uint8	colorsize;		// Byte 7
	0,	//	uint8	originxl;		// Byte 8
	0,	//	uint8	originxh;		// Byte 9
	0,	//	uint8	originyl;		// Byte 10
	0,	//	uint8	originyh;		// Byte 11
	0,	//	uint8	widthl;			// Byte 12
	0,	//	uint8	widthh;			// Byte 13
	0,	//	uint8	heightl;		// Byte 14
	0,	//	uint8	heighth;		// Byte 15
	32,	//	uint8	bpp;			// Byte 16
	0,	//	uint8	idesc;			// Byte 17
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
		pixel32	*s = bop->rgba;
		uint8	*d = BRGABuffer;
		long	 count = bop->width * bop->height;

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


