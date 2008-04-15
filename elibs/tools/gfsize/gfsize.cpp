/*************************************************************************
 *                                                                       *
 *                              GFSIZE.CPP                               *
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
		Pads or crops an image to be the specified width and height.
      Preserves original upper left corner (i.e. pads/crops right and bottoms sides), but
      can be told to preserve a different corner. 
 
   PROGRAMMERS
      Juan M. Alvarado		
 
   FUNCTIONS
 
   TABS : 4 7
 
   HISTORY
		10/30/96 : Created.
      
   TODO

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include <limits.h>
#include <echidna\ensure.h>

#include <echidna\argparse.h>
#include <echidna\readgfx.h>
#include <echidna\eerrors.h>
#include <echidna\eio.h>
#include <echidna\checkglu.h>
#include <echidna\gff.h>
#include <echidna\memsafe.h>
#include <echidna\utils.h>
#include <echidna\dbmess.h>

/*************************** C O N S T A N T S ***************************/

#define ENTRY_INDEX_MAX  255

/******************************* T Y P E S *******************************/

typedef enum {
   tkNone,     // No transparency in this image
   tkAlphaLow,  // Transparent if Alpha below given threshhold
   tkAlphaHigh,  // Transparent if Alpha above given threshhold
   tkRGB       // Transparent if Red, Green and Blue component equal given values.
} TRANSPARENCYKIND;
 
/************************** P R O T O T Y P E S **************************/

void CheckIntRangeNoFail (int Val, char *pszName, int MinVal, int MaxVal);
void CheckUint32RangeNoFail (UINT32 Val, char *pszName, UINT32 MinVal, UINT32 MaxVal);
void CopyMemRect (
   UINT8 *pu8BufDst, int WidthBufDst, int HeightBufDst, 
   UINT8 *pu8BufSrc, int WidthBufSrc, int HeightBufSrc,
   int xDst, int yDst, 
   int xSrc, int ySrc, 
   int WidthCopy, int HeightCopy
);

/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/


/**************************** R O U T I N E S ****************************/



/*************************** ArgParse Template ***************************/
enum {
   NDX_InFile,
   NDX_OutFile,
   NDX_Width,
   NDX_Height,
   NDX_Corner,
   NDX_Index,
   NDX_TransparentColor,
   NDX_TransparentAlpha,
   NDX_Quiet,
};

#define ARG(name) (newargs [NDX_ ## name])
#define qprintf(arg_list)  (!ARG(Quiet)) ? EL_printf arg_list : NULL

#define ARG_INFILE		(newargs[ 0])
#define ARG_OUTFILE		(newargs[ 1])

char Usage[] = "Usage: GFSize INFILE OUTFILE [Switches]\n";
static char	**newargs;

ArgSpec Template[] = {
   {STANDARD_ARG|REQUIRED_ARG,				"INFILE",	
      "\tINFILE  = GFF File to read\n", },
   {STANDARD_ARG|REQUIRED_ARG,				"OUTFILE",	
      "\tOUTFILE = GFF File to write\n", },
   {CHRKEYWORD_ARG,              "W",	   
      "\t-W<width>    Width to make image canvass. Default = original width.\n"
      "\t             Will be cropped or padded as necessary.\n"
   ,},
   {CHRKEYWORD_ARG,              "H",	   
      "\t-H<height>   Height to make image canvas. Default = original height.\n"
      "\t             Will be cropped or padded as necessary.\n"
   ,},
   {CHRKEYWORD_ARG,              "C",	   
      "\t-C<0..3>    Corner to preserve when cropping/paddding:\n"
      "\t                 0 = Top Left (default)\n"
      "\t                 1 = Top Right\n"
      "\t                 2 = Bottom Left\n"
      "\t                 3 = Bottom Right\n"
   ,},
   {CHRKEYWORD_ARG,              "I",	   
      "\t-I<0..255>    Index value for padding (paletted image). Default = 0.\n"
   ,},
   {KEYWORD_ARG, "-TC=TC",	      
      "\tTC<r:g:b>      Transparency by r,g,b value\n"
   , },
   {KEYWORD_ARG, "TA",	      
      "\t-TA<alpha>     Transparency by alpha value.\n"
      "                      If alhpa >= 0 then transparent values <= alpha.\n"
      "                      If alhpa <  0 then transparent values >= -alpha.\n"
   ,},      
   {CHRSWITCH_ARG, "Q",        
      "\t-Q             Quiet. No progress printing.\n"
   ,},
   {0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
BEGINFUNCMAIN(main)
{
	newargs = argparse (argc, argv, Template);

	if (!newargs)
	{
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		RETURN EXIT_FAILURE;
	}
	else
	{
      GFF	*pgff;
   
      pgff = ReadGFF (ARG(InFile));
      if (!pgff) 
      {
         EL_printf ("ERROR: Unable to read file '%s' as GFF file.\n", ARG(InFile));
         RETURN EXIT_FAILURE;
      }
      else
      {
         UINT32 WidthOld, HeightOld;
         UINT32 WidthNew, HeightNew;
         int Corner;
         CHUNKNODE *pchunknodePNDX;
         CHUNKNODE *pchunknodeRGBA;
         int PixelSize;
         UINT32 u32PixelDataSizeNew;
         CHUNKNODE *pchunknodePixelsOld;
         CHUNKNODE *pchunknodePixelsNew;
         CHUNKGENERIC   *pchunkgenericPixelsNew;
         IDTYPE idNewChunk;
         UINT8 *pu8PixelDataOld;
         UINT8 *pu8PixelDataNew;
         UINT8 *pu8PixelDataPad;
  		  TRANSPARENCYKIND tkTransparency;    // What kind of transparency to use.
		  int RedT, GreenT,BlueT;             // Transparency RGB values.
		  int AlphaT;                         // Transparency Alpha threshhold.
         
          ENSURE_(!(ARG(TransparentColor) && ARG(TransparentAlpha)), "Cannot use transparency by alpha and rgb at same time.");
		   tkTransparency = tkNone;
		   if (ARG(TransparentColor))
		   {
			  int result;
			  result = sscanf (ARG(TransparentColor), "%d:%d:%d", &RedT, &GreenT, &BlueT);               
			  //qprintf(("Transparency Red=%d, G=%d, B=%d\n", RedT, GreenT, BlueT));               
			  ENSURE_(3 == result, "Must provide all three components for r,g,b transprency. (no spaces)");
			  ENSURE(0 <= RedT &&  RedT <=ENTRY_INDEX_MAX);
			  ENSURE(0 <= GreenT &&  GreenT <=ENTRY_INDEX_MAX);
			  ENSURE(0 <= BlueT &&  BlueT <=ENTRY_INDEX_MAX);
			  tkTransparency = tkRGB;
		   }
		   else if (ARG(TransparentAlpha))
		   {
			  AlphaT = atoi (ARG(TransparentAlpha));
			  tkTransparency = (AlphaT < 0) ? tkAlphaHigh : tkAlphaLow;
			  AlphaT = UTL_ABS (AlphaT);
		   }
		  
         if (!ARG(Width) && !ARG(Height))
         {
            EL_printf ("ERROR: Nothing to do. Must specify a width or height with -W or -H\n");
            RETURN EXIT_FAILURE;
         }
         WidthOld = pgff->pchunkggff->Data.Width;
         HeightOld = pgff->pchunkggff->Data.Height;
         
         WidthNew = (ARG(Width)) ? atol (ARG(Width)) : WidthOld;
         CheckUint32RangeNoFail (WidthNew, "Width", 1, UINT32MAX-1);
         HeightNew = (ARG(Height)) ? atol (ARG(Height)) : HeightOld;
         CheckUint32RangeNoFail (HeightNew, "Height", 1, UINT32MAX-1);
            
         Corner = (ARG(Corner)) ? atoi (ARG(Corner)) : 0;
         CheckIntRangeNoFail (Corner, "Corner", 0, 3);
         
         /* Determine size of pixel data for this image */
         pchunknodePNDX = PChunkNodeOfId (pgff, IDPNDX);
         pchunknodeRGBA = PChunkNodeOfId (pgff, IDRGBA);
         
         if (pchunknodePNDX)
         {
            static UINT8 u8PadPixel;
            CHUNKPNDX *pchunkpndx;
            int PadIndex;
            
            pchunknodePixelsOld = pchunknodePNDX;
            pchunkpndx = (CHUNKPNDX *)pchunknodePNDX->pchunk;                                
            PixelSize = 1;   
            pu8PixelDataOld = (UINT8 *)&pchunkpndx->Data;
            idNewChunk = IDPNDX;
            PadIndex = (ARG(Index)) ? atoi(ARG(Index)) : 0;
            CheckIntRangeNoFail(PadIndex, "Index", 0, 255);
            u8PadPixel = (UINT8)PadIndex;
            pu8PixelDataPad = &u8PadPixel;
            
         }
         else if (pchunknodeRGBA)
         {
            static RGBADATA rgbadataPadPixel;
            CHUNKRGBA *pchunkrgba;
            int R, G, B, A;
            
            pchunknodePixelsOld = pchunknodeRGBA;
            pchunkrgba = (CHUNKRGBA *)pchunknodeRGBA->pchunk;                                
            PixelSize = 4;
            pu8PixelDataOld = (UINT8 *)&pchunkrgba->Data;
            idNewChunk = IDRGBA;
			 R = G = B = A = 0;
			  switch (tkTransparency) {
			  case tkNone:
				 break;
			  case tkAlphaLow:
				 break;
			  case tkAlphaHigh:
				 A = 255;
				 break;
			  case tkRGB:
				 R = RedT; G = GreenT; B = BlueT;
				 break;
			  default:
				 ENSURE(0);
				 break;   	 
				 
			  }
            CheckIntRangeNoFail (R, "Red", 0, 255);
            CheckIntRangeNoFail (G, "Green", 0, 255);
            CheckIntRangeNoFail (B, "Blue", 0, 255);
            CheckIntRangeNoFail (A, "Alpha", 0, 255);
            rgbadataPadPixel.Red = (UINT8) R;
            rgbadataPadPixel.Green = (UINT8) G;
            rgbadataPadPixel.Blue = (UINT8) B;
            rgbadataPadPixel.Alpha = (UINT8) A;
            pu8PixelDataPad = (UINT8 *)&rgbadataPadPixel;
         }
         else
         {
            EL_printf ("ERROR: Cannot find pixel data in GFF file '%s'.\n", ARG(InFile));
            RETURN EXIT_FAILURE;
         }
         
         /* Make new chunk for new image */
         u32PixelDataSizeNew = PixelSize * WidthNew * HeightNew;
         
         pchunknodePixelsNew = CreateGFFChunkNodeNoFail (idNewChunk, u32PixelDataSizeNew);
         pchunkgenericPixelsNew = (CHUNKGENERIC *)pchunknodePixelsNew->pchunk;
         pu8PixelDataNew = &pchunkgenericPixelsNew->u8First;
         
         /* Clear to Pad value */
         {                  
            int i, j;
            UINT8 *pu8Dst;
            
            for (i = WidthNew * HeightNew, 
                  pu8Dst = pu8PixelDataNew; 
                  i; 
                  i--
            )
            {
               UINT8 *pu8Src;
               for (j = PixelSize, pu8Src = pu8PixelDataPad; j; j--)
               {
                  *pu8Dst++ = *pu8Src++;
               }
            }
         }
         
         /* Determine copy coordinates in source and dest and copy data */
         {
            int xDst, yDst, xSrc, ySrc;
            int CopyWidth, CopyHeight;
            
            CopyWidth = UTL_MIN(WidthOld, WidthNew);
            CopyHeight = UTL_MIN(HeightOld, HeightNew);
            switch (Corner) {
            case 0: /* Top Left */
               xSrc = 0;
               ySrc = 0;
               xDst = 0;
               yDst = 0;
               break;
            case 1: /* Top Right */
               xSrc = WidthOld - WidthNew;
               ySrc = 0;
               xDst = WidthNew - WidthOld;
               yDst = 0;
               break;
            case 2: /* Bottom Left */
               xSrc = 0;
               ySrc = HeightOld - HeightNew;
               xDst = 0;
               yDst = HeightNew - HeightOld;
               break;
            case 3: /* Bottom Right */
               xSrc = WidthOld - WidthNew;
               ySrc = HeightOld - HeightNew;
               xDst = WidthNew - WidthOld;
               yDst = HeightNew - HeightOld;
               break;
            default:
               EL_printf ("ERROR: Invalid Corner value: %d.\n", Corner);
               RETURN EXIT_FAILURE;
            }
            xSrc = UTL_MAX (0, xSrc);
            ySrc = UTL_MAX (0, ySrc);
            xDst = UTL_MAX (0, xDst);
            yDst = UTL_MAX (0, yDst);
  
            CopyMemRect (
               pu8PixelDataNew, WidthNew * PixelSize, HeightNew, 
               pu8PixelDataOld, WidthOld * PixelSize, HeightOld,
               xDst * PixelSize, yDst, 
               xSrc * PixelSize, ySrc, 
               CopyWidth * PixelSize, CopyHeight);

			 /* Make an alpha channel if transparency is by color */
           if (!pchunknodePNDX && pchunknodeRGBA &&  tkRGB == tkTransparency) 
			{
				 int i;
                RGBADATA *prgbadataSrcPixel;
				 
				 for (i = WidthNew * HeightNew, 
					   prgbadataSrcPixel = (RGBADATA *)pu8PixelDataNew; 
					   i; 
					   i--, prgbadataSrcPixel++
				 )
				 {
					 prgbadataSrcPixel->Alpha = 255; // assume opaque
					 if (prgbadataSrcPixel->Red == RedT 
						 && prgbadataSrcPixel->Green == GreenT
						 && prgbadataSrcPixel->Blue == BlueT)
					{
						 prgbadataSrcPixel->Alpha = 0;	// change to transparent
					}
				 }
				 
			}
			    	  
            LST_Remove (pchunknodePixelsOld);
            DestroyGFFChunkNode (pchunknodePixelsOld);
            LST_AddTail (pgff->plistChunkNodes, pchunknodePixelsNew);
            pgff->pchunkggff->Data.Width = (UINT16)WidthNew;
            pgff->pchunkggff->Data.Height = (UINT16)HeightNew;
         }
         
         WriteGFF (ARG(OutFile), pgff);      
      }
   }
   RETURN EXIT_SUCCESS;
} 
ENDFUNCMAIN(main)

/*************************************************************************
                            CheckIntRangeNoFail                             
 *************************************************************************

   SYNOPSIS
		void CheckIntRangeNoFail (int Val, char *pszName, int MinVal, int MaxVal)

   PURPOSE
     	Check that value is in range and if not print message then exit program.
  
   INPUT
		Val     : Value to check.
		pszName : Name of value to print in error message.
		MinVal  : Minimum legal value of Val.
		MaxVal  : Maximum legal value of Val.
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   SEE ALSO
  
  
   HISTORY
		10/30/96 : Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CheckIntRangeNoFail (int Val, char *pszName, int MinVal, int MaxVal)
BEGINPROC (CheckIntRangeNoFail)
{
   if (Val < MinVal || Val > MaxVal)
   {
      EL_printf ("ERROR: Value of %d for %s is out of the legal range of %d..%d.\n", 
         Val, pszName, MinVal, MaxVal);
      exit (1);
   }
   
} ENDPROC (CheckIntRangeNoFail)


void CheckUint32RangeNoFail (UINT32 Val, char *pszName, UINT32 MinVal, UINT32 MaxVal)
BEGINPROC (CheckIntRangeNoFail)
{
   if (Val < MinVal || Val > MaxVal)
   {
      EL_printf ("ERROR: Value of %lu for %s is out of the legal range of %lu..%lu.\n", 
         Val, pszName, MinVal, MaxVal);
      exit (1);
   }
   
} ENDPROC (CheckIntRangeNoFail)

/*************************************************************************
                               CopyMemRect                               
 *************************************************************************

   SYNOPSIS
		void CopyMemRect (
		   UINT8 *pu8BufDst, int WidthBufDst, int HeightBufDst, 
		   UINT8 *pu8BufSrc, int WidthBufSrc, int HeightBufSrc,
		   int xDst, int yDst, 
		   int xSrc, int ySrc, 
		   int WidthCopy, int HeightCopy
		)

   PURPOSE
  		To copy a rectangular area of source raster buffer to a location
      in a destination raster buffer.
  
   INPUT
		pu8BufDst    :
		WidthBufDst  :
		HeightBufDst :
		pu8BufSrc    :
		WidthBufSrc  :
		HeightBufSrc :
		xDst         :
		yDst         :
		xSrc         :
		ySrc         :
		WidthCopy    :
		HeightCopy   :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   SEE ALSO
  
  
   HISTORY
		10/30/96 : Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CopyMemRect (
   UINT8 *pu8BufDst, int WidthBufDst, int HeightBufDst, 
   UINT8 *pu8BufSrc, int WidthBufSrc, int HeightBufSrc,
   int xDst, int yDst, 
   int xSrc, int ySrc, 
   int WidthCopy, int HeightCopy
)
BEGINPROC (CopyMemRect)
{
   int dxMarginSrc, dxMarginDst;
   int x, y;
   UINT8 *pu8Src, *pu8Dst;
  
#if 0  
EL_printf ("WidthBufDst=%d, HeightBufDst=%d, WidthBufSrc=%d, HeightBufSrc=%d\n"
         "xDst=%d, yDst=%d, xSrc=%d, ySrc=%d, WidthCopy=%d, HeightCopy=%d\n",
         WidthBufDst, HeightBufDst, WidthBufSrc, HeightBufSrc,
         xDst, yDst, xSrc, ySrc, WidthCopy, HeightCopy);
#endif         
   
   ENSURE(xSrc >= 0 && (xSrc + WidthCopy) <= WidthBufSrc);
   ENSURE(ySrc >= 0 && (ySrc + HeightCopy) <= HeightBufSrc);
   ENSURE(xDst >= 0 && (xDst + WidthCopy) <= WidthBufDst);
   ENSURE(yDst >= 0 && (yDst + HeightCopy) <= HeightBufDst);
   
   dxMarginSrc = WidthBufSrc - WidthCopy;
   dxMarginDst = WidthBufDst - WidthCopy;
#if 0  
EL_printf ("dxMarginSrc=%d, dxMarginDst=%d\n", dxMarginSrc, dxMarginDst);
#endif

   for (
      y = HeightCopy, 
         pu8Src = pu8BufSrc + ySrc * WidthBufSrc + xSrc,
         pu8Dst = pu8BufDst + yDst * WidthBufDst + xDst;
      y; 
      y--, 
         pu8Src += dxMarginSrc, 
         pu8Dst += dxMarginDst
   )
   {
      for (x = WidthCopy; x; x--)
      {
         *pu8Dst++ = *pu8Src++;
      }
   }

} ENDPROC (CopyMemRect)

