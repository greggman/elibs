/*************************************************************************
 *                                                                       *
 *                              READGFX.CPP                              *
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
#include "echidna/dbmess.h"

#include "echidna/readgfx.h"
#include "echidna/strings.h"
#include "echidna/eio.h"
#include "echidna/checkglu.h"
#include "echidna/eerrors.h"

#include "readpcx.h"
#include "readpic.h"
#include "readtga.h"
#include "photoshp.h"
#include "echidna/gff.h"

/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/


/************************** P R O T O T Y P E S **************************/


/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/


/**************************** R O U T I N E S ****************************/

/*********************************************************************
 *
 * Read32BitPicture
 *
 * SYNOPSIS
 *		BlockO32BitPixels *Read32BitPicture (CString &filename, CString &ext)
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
BlockO32BitPixels *Read32BitPicture (const char* filename)
{
	BlockO32BitPixels	*b32 = 0;
	MEMFILE				*mf;

	mf = MEMFILE_Load (filename);
	if (mf)
	{
		b32 = (BlockO32BitPixels *)calloc(sizeof (BlockO32BitPixels),1);
	
		if (!stricmp (".psd", EIO_Ext(filename)))
		{
			InfoMess (("Loading Photoshop file %s\n", filename));
			if (!loadPhotoshop32Bit (b32, mf))
			{
				free(b32);
				b32 = 0;
			}
		}
		else if (!stricmp (".tga", EIO_Ext(filename)))
		{
			InfoMess (("Loading Targa file %s\n", filename));
			if (!loadTGA32Bit (b32, mf))
			{
				free(b32);
				b32 = 0;
			}
		}
		else if (!stricmp (".pic", EIO_Ext(filename)))
		{
			InfoMess (("Loading Softimage PIC file %s\n", filename));
			if (!loadPIC32Bit (b32, mf))
			{
				free(b32);
				b32 = 0;
			}
		}
		else if (!stricmp (".pcx", EIO_Ext(filename)))
		{
			InfoMess (("Loading PCX file %s\n", filename));
			if (!loadPCX32Bit (b32, mf))
			{
				free(b32);
				b32 = 0;
			}
		}
		else if (!stricmp (".gff", EIO_Ext(filename)))
		{
			InfoMess (("Loading GFF file %s\n", filename));
			if (!loadGFF32Bit (b32, mf))
			{
				free(b32);
				b32 = 0;
			}
		}
		else
		{
			b32 = 0;
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("Unsupported file type '%s'", filename);
		}

		MEMFILE_Close (mf);
	}
	else
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf1 ("Trouble reading file '%s'", filename);
	}

	if (b32)
	{
		InfoMess (("Width = %d, Height = %d\n", b32->width, b32->height));
	}

	return b32;
}
// Read32BitPicture

void Free32BitPicture (BlockO32BitPixels *pBOP)
{
	if (pBOP->rgba)
	{
		free (pBOP->rgba);
	}

	free (pBOP);
}

/*************************************************************************
                            Write32BitPicture
 *************************************************************************

   SYNOPSIS
		int Write32BitPicture (const char *filename, BlockO32BitPixels *pBOP)

   PURPOSE


   INPUT
		char :
		pBOP :

   OUTPUT
		None

   EFFECTS
		None

   RETURNS


   SEE ALSO


   HISTORY
		07/15/96 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int Write32BitPicture (const char *filename, BlockO32BitPixels *pBOP)
BEGINFUNC (Write32BitPicture)
{
	int	fh;

	if (!stricmp(".tga", EIO_Ext(filename)))
	{
		fh = CHK_WriteOpen (filename);
		saveTGA32Bit (fh, pBOP);
		CHK_Close (fh);
	}
	else if (!stricmp(".gff", EIO_Ext(filename)))
	{
		fh = CHK_WriteOpen (filename);
		saveGFF32Bit (fh, pBOP);
		CHK_Close (fh);
 	}
	else if (!stricmp(".rgb", EIO_Ext(filename)))
	{
      long int i;
      long int pixels;
      pixel32 *ppixel32;

		fh = CHK_WriteOpen (filename);
      pixels = pBOP->width * pBOP->height;
      for (i = 0, ppixel32 = pBOP->rgba; i < pixels; i++, ppixel32++)
      {
         CHK_Write(fh, &ppixel32->red, 1);
         CHK_Write(fh, &ppixel32->green, 1);
         CHK_Write(fh, &ppixel32->blue, 1);
      }
		CHK_Close (fh);
 	}
	else
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf1 ("Unsupported 32bit save format '%s'", filename);
		return FALSE;
	}

	return TRUE;

} ENDFUNC (Write32BitPicture)

/*********************************************************************
 *
 * Read8BitPicture
 *
 * SYNOPSIS
 *		BlockO8BitPixels *Read8BitPicture (const char *filename)
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
BlockO8BitPixels *Read8BitPicture (const char* filename)
{
	BlockO8BitPixels	*b8 = 0;
	MEMFILE				*mf;

	mf = MEMFILE_Load (filename);
	if (mf)
	{
		b8 = (BlockO8BitPixels *)calloc(sizeof (BlockO8BitPixels),1);
	
		if (!stricmp (".psd", EIO_Ext(filename)))
		{
			#if 1
			InfoMess (("8 Bit Photoshop File not yet supported\n"));
			#else
			InfoMess (("Loading Photoshop file %s\n", filename));
			if (!loadPhotoshop8Bit (b8, mf))
			{
				free(b8);
				b8 = 0;
			}
			#endif
		}
		else if (!stricmp (".tga", EIO_Ext(filename)))
		{
			#if 1
			InfoMess (("8 Bit Targa File not yet supported\n"));
			#else
			InfoMess (("Loading Targa file %s\n", filename));
			if (!loadTGA8Bit (b8, mf))
			{
				free(b8);
				b8 = 0;
			}
			#endif
		}
		else if (!stricmp (".pcx", EIO_Ext(filename)))
		{
			InfoMess (("Loading PCX file %s\n", filename));
			if (!loadPCX8Bit (b8, mf))
			{
				free(b8);
				b8 = 0;
			}
		}
		else
		{
			b8 = 0;
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("Unsupported file type '%s'", filename);
		}

		MEMFILE_Close (mf);
	}
	else
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf1 ("Trouble reading file '%s'", filename);
	}

	if (b8)
	{
		InfoMess (("Width = %d, Height = %d\n", b8->width, b8->height));
	}

	return b8;
}
// Read8BitPicture

void Free8BitPicture (BlockO8BitPixels *pBOP)
{
	if (pBOP->pixels)
	{
		free (pBOP->pixels);
	}

	free (pBOP);
}


/*********************************************************************
 *
 * ReadRawPalette
 *
 * SYNOPSIS
 *    UINT8 *ReadPalette (const char* filename, int *pNumColors)
 *
 * PURPOSE
 *		To read just the palette from a paletted image file.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *    Pointer to allocated 256 entry palette on success. NULL on failure.
 *    Fills in *pNumColors with number of colors that were in file.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
UINT8 *ReadRawPalette (const char* filename, int *pNumColors)
{
	UINT8	*pu8 = NULL;
	MEMFILE				*mf;

	mf = MEMFILE_Load (filename);
	if (mf)
	{
		pu8 = (UINT8 *)calloc(4, 256);
	
		if (!stricmp (".raw", EIO_Ext(filename))
		 || !stricmp (".act", EIO_Ext(filename))
       )
		{
         int i;
         UINT8 *pu8Temp;
			InfoMess (("Loading raw palette %s\n", filename));
         for (i = 0, pu8Temp = pu8; i < 256; i++, pu8Temp += 4)
         {
            int result;
            result = MEMFILE_Read (mf, pu8Temp, 3);
            if (result != 3) break;
            pu8Temp[3] = 255;
         }
         *pNumColors = i;
		}
		else if (!stricmp (".tga", EIO_Ext(filename)))
		{
			#if 1
			InfoMess (("Targa File palette not yet supported\n"));
			#else
			InfoMess (("Loading Targa palette %s\n", filename));
			if (!loadTGAPalette (pu8, mf, pNumColors))
			{
				free(pu8);
				pu8 = 0;
			}
			#endif
		}
		else if (!stricmp (".pcx", EIO_Ext(filename)))
		{
	      BlockO8BitPixels	*pbop8;

			InfoMess (("Loading PCX palette %s\n", filename));
         pbop8 = Read8BitPicture (filename);
         if (pbop8)
         {
            int i;
            UINT8 *pu8Temp;
            paletteEntry *ppal;

   			InfoMess (("Copying PCX palette %s\n", filename));
            for (i = 0, pu8Temp = pu8, ppal = pbop8->palette; i < 256; i++, pu8Temp += 4, ppal++)
            {
               pu8Temp[0] = ppal->red;
               pu8Temp[1] = ppal->green;
               pu8Temp[2] = ppal->blue;
               pu8Temp[4] = 255;
            }
            Free8BitPicture (pbop8);
         }
         else
			{
				free(pu8);
				pu8 = 0;
			}
		}
      else if (!stricmp (".gff", EIO_Ext(filename)))
      {
         CHUNKHEADER   chunkheader;
         int result;

         /* Read chunks until find pcon chunk */
         for (;;)
         {
            result = MEMFILE_Read (mf, &chunkheader, sizeof (CHUNKHEADER));
            if (sizeof (CHUNKHEADER) != result || IDPCON == chunkheader.id || IDPCN2 == chunkheader.id)
            {
               break;
            } else  // skip unwanted data
            {
               MEMFILE_Seek (mf, chunkheader.Size, SEEK_CUR);
            }
         }

         if (sizeof (CHUNKHEADER) == result && chunkheader.id == IDPCON)
         { /* Found pcon chunk */
            int i;
            UINT8 *pu8Temp;

            for (i = 0, pu8Temp = pu8; i < 256; i++, pu8Temp += 4)
            {
               int result;
               PCONDATA pcondata;
               result = MEMFILE_Read (mf, &pcondata, sizeof(PCONDATA));
               if (result != sizeof(PCONDATA)) break;
               pu8Temp[0] = pcondata.Red;
               pu8Temp[1] = pcondata.Green;
               pu8Temp[2] = pcondata.Blue;
               pu8Temp[3] = 255;
            }
            *pNumColors = i;
         }
         else if (sizeof (CHUNKHEADER) == result && chunkheader.id == IDPCN2)
         { /* Found pcon chunk */
            int i;
            UINT8 *pu8Temp;

            for (i = 0, pu8Temp = pu8; i < 256; i++, pu8Temp += 4)
            {
               int result;
               PCN2DATA pcondata;
               result = MEMFILE_Read (mf, &pcondata, sizeof(PCN2DATA));
               if (result != sizeof(PCN2DATA)) break;
               pu8Temp[0] = pcondata.Red;
               pu8Temp[1] = pcondata.Green;
               pu8Temp[2] = pcondata.Blue;
               pu8Temp[3] = pcondata.Alpha;
            }
            *pNumColors = i;
         }
      }
		else
		{
			pu8 = 0;
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("Unsupported palette file type '%s'", filename);
		}

		MEMFILE_Close (mf);
	}
	else
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf1 ("Trouble reading palette file '%s'", filename);
	}

	if (pu8)
	{
		InfoMess (("NumColors = %d\n", *pNumColors));
	}

	return pu8;
}
// ReadPalette

void FreeRawPalette (UINT8 *pu8Palette)
{
   ENSURE_PTR (pu8Palette);
	free (pu8Palette);
}

/*************************************************************************
                            Write8BitPicture
 *************************************************************************

   SYNOPSIS
		int Write8BitPicture (const char *filename, BlockO8BitPixels *pBOP)

   PURPOSE


   INPUT
		char :
		pBOP :

   OUTPUT
		None

   EFFECTS
		None

   RETURNS


   SEE ALSO


   HISTORY
		07/15/96 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int Write8BitPicture (const char *filename, BlockO8BitPixels *pBOP)
BEGINFUNC (Write8BitPicture)
{
	int	fh;

	if (!stricmp(".pcx", EIO_Ext(filename)))
	{
		fh = CHK_WriteOpen (filename);
		SavePCX8Bit (fh, pBOP);
		CHK_Close (fh);
	}
	else
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf1 ("Unsupported 8 bit save format '%s'", filename);
		return FALSE;
	}

	return TRUE;

} ENDFUNC (Write8BitPicture)



/*********************************************************************
 *
 * ReadGrey8BitPicture
 *
 * SYNOPSIS
 *		BlockOGrey8BitPixels *ReadGrey8BitPicture (const char *filename)
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
BlockOGrey8BitPixels *ReadGrey8BitPicture (const char* filename)
{
	BlockOGrey8BitPixels	*b8 = 0;
	MEMFILE				*mf;

	mf = MEMFILE_Load (filename);
	if (mf)
	{
		b8 = (BlockOGrey8BitPixels *)calloc(sizeof (BlockOGrey8BitPixels),1);
	
		if (!stricmp (".psd", EIO_Ext(filename)))
		{
			#if 1
			InfoMess (("8 Bit Photoshop File not yet supported\n"));
			#else
			InfoMess (("Loading Photoshop file %s\n", filename));
			if (!loadPhotoshopGrey8Bit (b8, mf))
			{
				free(b8);
				b8 = 0;
			}
			#endif
		}
		else if (!stricmp (".tga", EIO_Ext(filename)))
		{
			#if 1
			InfoMess (("8 Bit Targa File not yet supported\n"));
			#else
			InfoMess (("Loading Targa file %s\n", filename));
			if (!loadTGAGrey8Bit (b8, mf))
			{
				free(b8);
				b8 = 0;
			}
			#endif
		}
		else
		{
			b8 = 0;
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("Unsupported file type '%s'", filename);
		}

		MEMFILE_Close (mf);
	}
	else
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf1 ("Trouble reading file '%s'", filename);
	}

	if (b8)
	{
		InfoMess (("Width = %d, Height = %d\n", b8->width, b8->height));
	}

	return b8;
}
// ReadGrey8BitPicture

void FreeGrey8BitPicture (BlockOGrey8BitPixels *pBOP)
{
	if (pBOP->pixels)
	{
		free (pBOP->pixels);
	}

	free (pBOP);
}

/*************************************************************************
                            WriteGrey8BitPicture
 *************************************************************************

   SYNOPSIS
		int WriteGrey8BitPicture (const char *filename, BlockOGrey8BitPixels *pBOP)

   PURPOSE


   INPUT
		char :
		pBOP :

   OUTPUT
		None

   EFFECTS
		None

   RETURNS


   SEE ALSO


   HISTORY
		07/15/96 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int WriteGrey8BitPicture (const char *filename, BlockOGrey8BitPixels *pBOP)
BEGINFUNC (WriteGrey8BitPicture)
{
	int	fh;

	if (!stricmp(".tga", EIO_Ext(filename)))
	{
		fh = CHK_WriteOpen (filename);
		saveTGAGrey8Bit (fh, pBOP);
		CHK_Close (fh);
	}
	else
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf1 ("Unsupported 8 bit save format '%s'", filename);
		return FALSE;
	}

	return TRUE;

} ENDFUNC (WriteGrey8BitPicture)



