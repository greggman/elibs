/*************************************************************************
 *                                                                       *
 *                             GFSHRINK.CPP                              *
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
      Scales GFF image size downward.


   PROGRAMMERS
      Juan M. Alvarado

   FUNCTIONS

   TABS : 4 7

   HISTORY
		08/01/96 : JMA Created.

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include <echidna\ensure.h>

#include <math.h>
#include <echidna\argparse.h>
#include <echidna\readgfx.h>
#include <echidna\eerrors.h>
#include <echidna\eio.h>
#include <echidna\checkglu.h>
#include <echidna\gff.h>
#include <echidna\memsafe.h>
#include <echidna\utils.h>

/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/


/************************** P R O T O T Y P E S **************************/

typedef enum {
   tkNone,     // No transparency in this image
   tkAlphaLow,  // Transparent if Alpha below given threshhold
   tkAlphaHigh,  // Transparent if Alpha above given threshhold
   tkRGB       // Transparent if Red, Green and Blue component equal given values.
} TRANSPARENCYKIND;

void ScaleGFF (
   int Width,
   int Height,
   RGBADATA *prgbaData,
   int xin,
   int xout,
   int yin,
   int yout,
   RGBADATA *prgbaDataNew
);

UINT32 NewSizeOfScaledRGBA (
   int Width,
   int Height,
   int xin,
   int xout,
   int yin,
   int yout,
   int *pWidthNew,
   int *pHeightNew
);

/***************************** G L O B A L S *****************************/

TRANSPARENCYKIND tkTransparency;    // What kind of transparency to use.
int RedT, GreenT,BlueT;             // Transparency RGB values.
int AlphaT;                         // Transparency Alpha threshhold.

/****************************** M A C R O S ******************************/

#define round(a)  floor((a) + 0.5)

/**************************** R O U T I N E S ****************************/


/*************************** ArgParse Template ***************************/
#define ARG_INFILE		(newargs[ 0])
#define ARG_OUTFILE		(newargs[ 1])
#define ARG_Q           (newargs[ 2])
#define ARG_X           (newargs[ 3])
#define ARG_Y           (newargs[ 4])
#define ARG_TC			 (newargs[ 5])
#define ARG_TA			 (newargs[ 6])

char Usage[] = "Usage: GFShrink INFILE OUTFILE\n";

ArgSpec Template[] = {
	 {STANDARD_ARG|REQUIRED_ARG,   "INFILE",	"\tINFILE          Binary File to read\n", },
	 {STANDARD_ARG|REQUIRED_ARG,   "OUTFILE",	"\tOUTFILE         Binary File to write\n", },
	 {CHRSWITCH_ARG,               "Q",        "\t-Q              Quiet. No progress printing.\n",},
	 {CHRKEYWORD_ARG,              "X",	      "\t-X<out>[/<in>]  Shrink factor for width.  Use 0 for <in> or <out> to\n"
															"\t              mean original width.\n", },
	 {CHRKEYWORD_ARG,              "Y",	      "\t-Y<out>[/<in>]  Shrink factor for height. Use 0 for <in> or <out> to\n"
											   "\t                mean original height.\n"
											   "\n"
											   "One of -X or -Y must be specified. If only one is specified then the other will default "
											   "to the same factor. To keep one dimension unchanged use 0 (e.g. -X0 or -X0/0) or any factor "
											   "of 1 (e.g. 1/1 2/2)\n"
											   "\n"
	 , },
   {KEYWORD_ARG, "-TC=TC",	      
      "    TC<r:g:b>      Transparency by r,g,b value\n"
   , },
	 {KEYWORD_ARG, "TA",	      
      "    -TA<alpha>     Transparency by alpha value.\n"
      "                      If alhpa >= 0 then transparent values <= alpha.\n"
      "                      If alhpa <  0 then transparent values >= -alpha.\n"
	 ,},      
	 {0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
BEGINFUNCMAIN(main)
{
	char			**newargs;
   
	newargs = argparse (argc, argv, Template);

	if (!newargs)
	{
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		return EXIT_FAILURE;
	}
	else
	{
		// Load a 32 bit picture (or 8bit as 32bit) and then save it
		{
			 GFF	*pgff;

			pgff = ReadGFF (ARG_INFILE);
			if (pgff)
			{
            // in out pixel ratio values
            int xin, xout; // xout/xin == scale factor for width 
            int yin, yout; // yout/yin == scale factor for height
            CHUNKNODE   *pchunknodeRGBANew;  // pointer to new chunknode for scaled image.
            CHUNKRGBA   *pchunkrgbaNew;      // pointer to new chunk for scaled image.
            int NewWidth, NewHeight;         // New dimensions of scaled image.

            ENSURE_PTR(pgff->pchunkggff);
            ENSURE_PTR(pgff->pchunkrgba);
            if (!ARG_Q) EL_printf("Input Size = %d X %d\n", pgff->pchunkggff->Data.Width, pgff->pchunkggff->Data.Height);

			 /* Get transparency options */
			   ENSURE_(!(ARG_TC && ARG_TA), "Cannot use transparency by alpha and rgb at same time.");
			   tkTransparency = tkNone;
			   if (ARG_TC)
			   {
				  int result;
				  result = sscanf (ARG_TC, "%d:%d:%d", &RedT, &GreenT, &BlueT);               
				  if (!ARG_Q) EL_printf("Transparency Red=%d, G=%d, B=%d\n", RedT, GreenT, BlueT);               
				  ENSURE_(3 == result, "Must provide all three components for r,g,b transprency. (no spaces)");
				  ENSURE(0 <= RedT &&  RedT <=255);
				  ENSURE(0 <= GreenT &&  GreenT <=255);
				  ENSURE(0 <= BlueT &&  BlueT <=255);
				  tkTransparency = tkRGB;
			   }
			   else if (ARG_TA)
			   {
				  AlphaT = atoi (ARG_TA);
				  tkTransparency = (AlphaT < 0) ? tkAlphaHigh : tkAlphaLow;
				  AlphaT = UTL_ABS (AlphaT);
			   }

            xin = xout = yin = yout = 0;
            // Parse X and Y ratios.
            if (ARG_X)
            {
               sscanf (ARG_X,"%d/%d", &xout, &xin);
            }
            if (ARG_Y)
            {
               sscanf (ARG_Y,"%d/%d", &yout, &yin);
            }

            // Default zero-valued entries to original dimensions of image.
            if (!xin) xin = pgff->pchunkggff->Data.Width;
            if (!xout) xout = pgff->pchunkggff->Data.Width;
            if (!yin) yin = pgff->pchunkggff->Data.Height;
            if (!yout) yout = pgff->pchunkggff->Data.Height;

            // Default unspecified dimension to same ratios as specified dimension
            if (ARG_X && !ARG_Y)
            {
               yin = xin;
               yout = xout;
            }
            else if (ARG_Y && !ARG_X)
            {
               xin = yin;
               xout = yout;
            }
            if (!ARG_Q)EL_printf("Scale Factors: X=%d/%d  Y=%d/%d\n", xout, xin, yout, yin);

            // Make sure legal values specified for X and Y
            if (xin < xout)
            {
               EL_printf ("ERROR: X <out> must be smaller or equal to X <in>.\n");
               RETURN EXIT_FAILURE;
            }
            if (yin < yout)
            {
               EL_printf ("ERROR: Y <out> must be smaller or equal to Y <in>.\n");
               RETURN EXIT_FAILURE;
            }
            if (xin == xout && yin == yout)
            {
               EL_printf ("ERROR: Must specify a change in width or height with the -X or -Y parameters.\n");
               RETURN EXIT_FAILURE;
            }

            /*
            ** Allocate new RGBA chunk for scaled image data.
            */
            {
               UINT32 NewRGBADataSize;

               NewRGBADataSize = NewSizeOfScaledRGBA (
                  pgff->pchunkggff->Data.Width,
                  pgff->pchunkggff->Data.Height,
                  xin, xout, yin, yout, &NewWidth, &NewHeight);
               pchunknodeRGBANew = CreateGFFChunkNode (IDRGBA, NewRGBADataSize);
               pchunkrgbaNew = (CHUNKRGBA *)pchunknodeRGBANew->pchunk;
            }

            /*
            ** Scale the image.
            */
            ScaleGFF (
                  pgff->pchunkggff->Data.Width, pgff->pchunkggff->Data.Height,
                  &pgff->pchunkrgba->Data,
                  xin, xout, yin, yout,
                  &pchunkrgbaNew->Data
            );

            /*
            ** Replace old image data with new image data.
            */
            {
               CHUNKNODE *pchunknodeRGBAOld;

               // Remove old  RGBA chunk
               pchunknodeRGBAOld = PChunkNodeOfId (pgff, IDRGBA);
               ENSURE_PTR(pchunknodeRGBAOld);
               LST_Remove (pchunknodeRGBAOld);
               DestroyGFFChunkNode (pchunknodeRGBAOld);

               // Add new rgba chunk 
               LST_AddTail (pgff->plistChunkNodes, pchunknodeRGBANew);

               // Overwrite dimensions information in GGFF chunk with new dimensions
               pgff->pchunkggff->Data.Width = NewWidth;
               pgff->pchunkggff->Data.Height= NewHeight;
            }

            WriteGFF (ARG_OUTFILE, pgff);
            FreeGFF (pgff);
			}
		}
	}

	if (GlobalErr) {
		EL_printf ("%s\n", GlobalErrMsg);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
ENDFUNCMAIN(main)

/*************************************************************************
                                ScaleGFF
 *************************************************************************

   SYNOPSIS
		void ScaleGFF (
		   int Width,
         int Height,
         RGBADATA *prgbaData
		   int xin,
		   int xout,
		   int yin,
		   int yout,
         RGBADATA *prgbaDataNew
		)

   PURPOSE
      To create a scaled down version of the original image.

   INPUT
		Width        : Original width
		Height       : Original height
		prgbaData    : Pointer to RGBA data buffer of original image.
		xin          : xout/xin = scale factor for width.
		xout         : xout/xin = scale factor for width.
		yin          : yout/yin = scale factor for height.
		yout         : yout/yin = scale factor for height.
		prgbaDataNew : Pointer to RGBA data buffer for scaled image data.

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		08/05/96 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void ScaleGFF (
   int Width,
   int Height,
   RGBADATA *prgbaData,
   int xin,
   int xout,
   int yin,
   int yout,
   RGBADATA *prgbaDataNew
)
BEGINPROC (ScaleGFF)
{
   float dxStep, dyStep;  // How many pixels to step across in the source for every pixel in the dest.
   float dxyArea;          // Area in src of one destination pixel.
   float Red, Green, Blue, Alpha;   // Accumulators for pixel components.
   int   xDst, yDst;       // Loop indexes for destination image.
   RGBADATA *prgbadataDst;  // Destination data ptr.
   int   xDstMaxex, yDstMaxex;   // dimensions of new image.

   dxStep = (float)xin / (float)xout;
   dyStep = (float)yin / (float)yout;
   dxyArea = dxStep * dyStep;

   
   // Get size of new image
   NewSizeOfScaledRGBA (Width, Height, xin, xout, yin, yout, &xDstMaxex, &yDstMaxex);

   for (yDst = 0, prgbadataDst = prgbaDataNew;
      yDst < yDstMaxex;
      yDst++
   )
   {
      for (xDst = 0; xDst < xDstMaxex; xDst++, prgbadataDst++)
      {
         float xCrnt, xNext;     // xCrnt = index into leftmost position in source span.
                                 // xNext = index info leftmost position of next source span on same line.
         float yCrnt, yNext;
         int xSrc, ySrc;
         int xFirst, xLast;     // First and last src pixel to consider in generating dst pixel.
         int yFirst, yLast;
         RGBADATA *prgbaLineSrc;  // Source line ptr. 
         RGBADATA *prgbadataSrc;  // Source pixel data ptr.
         float NonPixels;		// Pixels off edge of image.
         float TransPixels;	// Transparent pixels

         /*
         ** Figure out start position of current and next dst pixels in 
         ** fractional src coordinates.
         */
         xCrnt = xDst * dxStep;
         xNext = xCrnt + dxStep;
         yCrnt = yDst * dyStep;
         yNext = yCrnt + dyStep;

         /*
         ** Figure out integer span of pixel indexes in src coordinates that covers all src 
         ** pixels that are part of the current dst pixel.
         */
         xFirst = (int)floor(xCrnt);
         xLast = (int)ceil(xNext) - 1;
         yFirst = (int)floor(yCrnt);
         yLast = (int)ceil(yNext) - 1;

         // Clear accumulators.
         Red = (float)0.0;
         Green = (float)0.0;
         Blue = (float)0.0;
         Alpha = (float)0.0;

         NonPixels = TransPixels = (float)0.0;
         // Walk over src pixels adding as much of them as is in the dst pixel to the accumulators.
         for ( ySrc = yFirst,
                  prgbaLineSrc = prgbaData +
                     (yFirst * Width) + xFirst,
                  prgbadataSrc = prgbaLineSrc;
               ySrc <= yLast;
               ySrc++,
                  prgbaLineSrc += Width,
                  prgbadataSrc = prgbaLineSrc
         )
         {
            float yAliasFactor;

            yAliasFactor = (float)1.0;
            if ((float)ySrc < yCrnt)
            {
               yAliasFactor = (float)ySrc + (float)1.0 - yCrnt;
            }
            else if ((float)ySrc + (float)1.0 >= yNext)
            {
               yAliasFactor = yNext - (float)ySrc;
            }

            for (xSrc = xFirst; xSrc <= xLast; xSrc++, prgbadataSrc++)
            {
               float xAliasFactor, AliasFactor;

               xAliasFactor = (float)1.0;
               if ((float)xSrc < xCrnt)
               {
                  xAliasFactor = (float)xSrc + (float)1.0 - xCrnt;
               }
               else if ((float)xSrc + (float)1.0 >= xNext)
               {
                  xAliasFactor = xNext - (float)xSrc;
               }
               AliasFactor = xAliasFactor * yAliasFactor;

               // Only add pixel if is in src image (i.e. not off right or bottom edge).
               if (xSrc < Width && ySrc < Height)
               {
					// And Only add non transparent pixels to color components.
					 switch (tkTransparency) {
					 case tkNone:
						break;
					 case tkAlphaLow:
						if (prgbadataSrc->Alpha <= AlphaT)
						{
						   TransPixels += AliasFactor;
						   Alpha += ((float)prgbadataSrc->Alpha)*AliasFactor;
						 continue;
						}
						break;
					 case tkAlphaHigh:
						if (prgbadataSrc->Alpha >= AlphaT) 
						{
						   TransPixels += AliasFactor;
						   Alpha += ((float)prgbadataSrc->Alpha)*AliasFactor;
						   continue;
						}
						break;
					 case tkRGB:
						#if 1
						if (prgbadataSrc->Red == RedT && prgbadataSrc->Green == GreenT && prgbadataSrc->Blue == BlueT) 
						{
						   TransPixels += AliasFactor;
						   Alpha += ((float)prgbadataSrc->Alpha)*AliasFactor;
						   continue;
						}
						#endif
						break;
					 } // Transparency kind switch
				
                  /* Add the AliasFactored RGBA values into accumulator */
                  Red += ((float)prgbadataSrc->Red)*AliasFactor;
                  Green += ((float)prgbadataSrc->Green)*AliasFactor;
                  Blue += ((float)prgbadataSrc->Blue)*AliasFactor;
                  Alpha += ((float)prgbadataSrc->Alpha)*AliasFactor;
               }
               else
               {
                  NonPixels += AliasFactor;
               }
            }
         }
         {
            float ActualArea, ActualAlphaArea;
			 ActualAlphaArea = dxyArea - NonPixels;
            ActualArea = ActualAlphaArea - TransPixels;
            prgbadataDst->Red   = (UINT8)round(Red   / ActualArea);
            prgbadataDst->Green = (UINT8)round(Green / ActualArea);
            prgbadataDst->Blue  = (UINT8)round(Blue  / ActualArea);
            prgbadataDst->Alpha = (UINT8)round(Alpha / ActualAlphaArea);
			 if (tkRGB == tkTransparency && prgbadataDst->Alpha <= (float)127.0)
			 {
				 prgbadataDst->Red   = (UINT8)RedT;
				 prgbadataDst->Green = (UINT8)GreenT;
				 prgbadataDst->Blue  = (UINT8)BlueT;
			 }
         }
      }
   }

} ENDPROC (ScaleGFF)

/*************************************************************************
                           NewSizeOfScaledRGBA
 *************************************************************************

   SYNOPSIS
		UINT32 NewSizeOfScaledRGBA (
		   int Width,
		   int Height,
		   int xin,
		   int xout,
		   int yin,
		   int yout,
		   int *pWidthNew,
		   int *pHeightNew
		)

   PURPOSE
      To determine the new dimensions and memory requirements of the scaled image.

   INPUT
		Width      : Original width
		Height     : Original height
		xin        : xout/xin == scale factor for width
		xout       : xout/xin == scale factor for width
		yin        : yout/yin == scale factor for height
		yout       : yout/yin == scale factor for height
		pWidthNew  : Pointer to New Width to fill in.
		pHeightNew : Pointer to new height to fill in.

   OUTPUT
		pWidthNew  : Pointer to New Width to fill in.
		pHeightNew : Pointer to new height to fill in.

   EFFECTS
		None

   RETURNS
      Size in bytes of memory required to store new rgba data for scaled image.

   SEE ALSO


   HISTORY
		08/13/96 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

UINT32 NewSizeOfScaledRGBA (
   int Width,
   int Height,
   int xin,
   int xout,
   int yin,
   int yout,
   int *pWidthNew,
   int *pHeightNew
)
BEGINFUNC (NewSizeOfScaledRGBA)
{
   UINT32   DataSize;      // Size of rgba data of new image.
   int   xDstMaxex, yDstMaxex;   // dimensions of new image.

   // Calculate size of new image
   xDstMaxex = (Width * xout + xin - 1) / xin;
   *pWidthNew = xDstMaxex;
   yDstMaxex = (Height * yout + yin - 1) / yin;
   *pHeightNew = yDstMaxex;

   DataSize = yDstMaxex * xDstMaxex * sizeof (RGBADATA);

	RETURN DataSize;
} ENDFUNC (NewSizeOfScaledRGBA)
