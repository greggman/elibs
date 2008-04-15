/*************************************************************************
 *                                                                       *
 *                               READGFX.H                               *
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

#ifndef EL_READGFX_H
#define EL_READGFX_H

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include "echidna/readgfx.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************** I N C L U D E S ****************************/


/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/

#if _EL_OS_WIN32__
	typedef struct
	{
		uint8	blue;
		uint8	green;
		uint8	red;
		uint8	alpha;
	}
	pixel32;
#else
	typedef struct
	{
		uint8	red;
		uint8	green;
		uint8	blue;
		uint8	alpha;
	}
	pixel32;
#endif

typedef struct
{
	uint8	red;
	uint8	green;
	uint8	blue;
}
paletteEntry;

typedef struct BlockO32BitPixels
{
	long	 width;
	long	 height;
	int		 channels;
	pixel32	*rgba;
} BlockO32BitPixels;

typedef struct BlockO8BitPixels
{
	long			 width;
	long			 height;
	paletteEntry	 palette[256];
	uint8			*pixels;
} BlockO8BitPixels;

typedef struct BlockOGrey8BitPixels
{
	long			 width;
	long			 height;
	uint8			*pixels;
} BlockOGrey8BitPixels;

/***************************** G L O B A L S *****************************/

/****************************** M A C R O S ******************************/

/************************** P R O T O T Y P E S **************************/

extern BlockO32BitPixels *Read32BitPicture (const char* filename);
extern int Write32BitPicture (const char* filename, BlockO32BitPixels *pBOP);
extern void Free32BitPicture (BlockO32BitPixels *pBOP);

extern BlockO8BitPixels *Read8BitPicture (const char* filename);
extern int Write8BitPicture (const char* filename, BlockO8BitPixels *pBOP);
extern void Free8BitPicture (BlockO8BitPixels *pBOP);

extern BlockOGrey8BitPixels *ReadGrey8BitPicture (const char* filename);
extern int WriteGrey8BitPicture (const char* filename, BlockOGrey8BitPixels *pBOP);
extern void FreeGrey8BitPicture (BlockOGrey8BitPixels *pBOP);

extern int flipBuffer (void *buffer, long rowSize, long rows);

extern UINT8 *ReadRawPalette (const char *filename, int *pNumColors);
extern void FreeRawPalette (UINT8 *pu8Palette);

#ifdef __cplusplus
}
#endif

#endif /* EL_READGFX_H */





