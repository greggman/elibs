/*************************************************************************
 *                                                                       *
 *                              GF16BIT.CPP                              *
 *                                                                       *
 *************************************************************************
 
                          Copyright 1996 Echidna                         
 
   DESCRIPTION
   
      To reduce a .GFF file from 32 bit color to 16 bit color, optionaly with 
      dithering.  
 
   PROGRAMMERS
		John Alvarado
 
   FUNCTIONS
 
   TABS : 4 7
 
   HISTORY
      11/01/96 : Created.

   TO DO
      Fix bug in error propagation: must not distribute error from or two
         transparent pixels.
 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
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


/******************************* T Y P E S *******************************/

typedef enum {
   dmMinex = -1,
   dmNone,
   dmErrorPropagation,
   dmOrdered,
   dmMaxex
} DITHERMETHOD;

typedef enum {
   tkNone,     // No transparency in this image
   tkAlphaLow,  // Transparent if Alpha below given threshhold
   tkAlphaHigh,  // Transparent if Alpha above given threshhold
   tkRGB       // Transparent if Red, Green and Blue component equal given values.
} TRANSPARENCYKIND;

typedef enum {
   trMinex = -1,
   trNone0,
   trNone1,
   trBlackAsBlue0,
   trBlackAsBlue1,
   trHighBit0,
   trHighBit1,
   trTrickHighBit0,
   trTrickHighBit1,
   trMaxex
} TRANSPARENCYRAW;

/************************** P R O T O T Y P E S **************************/

void ReduceWithNoDither (
   RGBADATA *prgbadataNew, 
   RGBADATA *prgbadataOld, 
   int Width, 
   int Height
);

void ReduceWithErrorPropagationDither (
   RGBADATA *prgbadataNew, 
   RGBADATA *prgbadataOld, 
   int Width, 
   int Height
);

void ReduceWithOrderedDither (
   RGBADATA *prgbadataNew, 
   RGBADATA *prgbadataOld, 
   int Width, 
   int Height
);

void WriteRaw16 (
   RGBADATA *prgbadata, 
   char *pszFile, 
   int Width, 
   int Height,
   TRANSPARENCYRAW tr
);

void DetermineTransparency (
   RGBADATA *prgbadataNew, 
   RGBADATA *prgbadataOld, 
   int Width, 
   int Height,
   TRANSPARENCYKIND tk,
   UINT8 Alpha,
   UINT8 Red,
   UINT8 Green,
   UINT8 Blue
);

/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/


/**************************** R O U T I N E S ****************************/

/*************************** ArgParse Template ***************************/
enum {
   NDX_InFile,
   NDX_OutFile,
   NDX_Dither,
   NDX_LittleEndian,
   NDX_Quiet,
   NDX_Raw,
   NDX_TransparentColor,
   NDX_TransparentAlpha,
   NDX_PixelOut,
};

#define ARG(name) (newargs [NDX_ ## name])
#define qprintf(arg_list)  (!ARG(Quiet)) ? EL_printf arg_list : NULL

char Usage[] = "Usage: GF16Bit INFILE OUTFILE [Switches]\n";
static char	**newargs;

ArgSpec Template[] = {
   {STANDARD_ARG|REQUIRED_ARG,				"INFILE",	
      "\tINFILE  = GFF File to read\n", },
   {STANDARD_ARG|REQUIRED_ARG,				"OUTFILE",	
      "\tOUTFILE = GFF File to write\n", },
   {CHRKEYWORD_ARG,              "D",	   
      "\t-D<method>    Dithering method to use:.\n"
      "\t                 0 = None (default).\n"
      "\t                 1 = Error Propogation (best for hi-res image).\n"
      "\t                 2 = Ordered (best for low-res image) (not implemented).\n"
   ,},
   {CHRSWITCH_ARG, "L",        
      "\t-L             Little endian output for Raw file. Default is big endian.\n"
   ,},
   {CHRSWITCH_ARG, "Q",        
      "\t-Q             Quiet. No progress printing.\n"
   ,},
   {CHRSWITCH_ARG, "R",        
      "\t-R             Output a raw 16 bit file for OUTFILE.\n"
   ,},
   {KEYWORD_ARG, "TC",	      
      "\tTC<r:g:b>     Transparency by r,g,b value.\n"
   , },
   {KEYWORD_ARG, "TA",	      
      "\tTA<alpha>     Transparency by alpha value.\n"
      "\t                  If alhpa >= 0 then transparent values <= alpha.\n"
      "\t                  If alhpa <  0 then transparent values >= -alpha.\n"
   ,},      
   {CHRKEYWORD_ARG, "P",	      
      "\t-P<type>      Pixel output type for Raw 16 bit output. Default=0\n"
      "\t                  0 = No Transparency with high bit 0:\n"
      "\t                              Bits = 0rrrrrgggggbbbbb.\n"
      "\t                  1 = No Transparency with high bit 1:\n"
      "\t                              Bits = 1rrrrrgggggbbbbb.\n"
      "\t                  2 = \"Black-as-Blue\" Transparency with high bit 0:\n"
      "\t                       Transparent = 0000000000000000,\n"
      "\t                            Opaque = 0rrrrrgggggbbbbb != 0x0000\n"
      "\t                            Black -> 0000000000000001 == 0x0001\n"
      "\t                  3 = \"Black-as-Blue\" Transparency with high bit 1:\n"
      "\t                       Transparent = 1000000000000000,\n"
      "\t                            Opaque = 1rrrrrgggggbbbbb != 0x8000\n"
      "\t                            Black -> 1000000000000001 == 0x8001\n"
      "\t                  4 = High Bit Clear Transparency:\n"
      "\t                       Transparent = 0rrrrrgggggbbbbb,\n"
      "\t                            Opaque = 1rrrrrgggggbbbbb,\n"
      "\t                  5 = High Bit Set Transparency:\n"
      "\t                       Transparent = 1rrrrrgggggbbbbb,\n"
      "\t                            Opaque = 0rrrrrgggggbbbbb,\n"
      "\t                  6 = High Bit 0 Tricky:\n"
      "\t                       Transparent = 0000000000000000,\n" 
      "\t                             Trick = 0rrrrrgggggbbbbb != 0x0000,\n"
      "\t                      Trick Black -> 0000000000000001 == 0x0001,\n"
      "\t                            Opaque = 1rrrrrgggggbbbbb,\n"
      "\t                  7 = High Bit 1 Tricky:\n"
      "\t                       Transparent = 1000000000000000,\n" 
      "\t                             Trick = 1rrrrrgggggbbbbb != 0x8000,\n"
      "\t                      Trick Black -> 1000000000000001 == 0x8001,\n"
      "\t                            Opaque = 0rrrrrgggggbbbbb,\n"
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
      DITHERMETHOD   dmDither;      
      TRANSPARENCYKIND tkTransparency;    // What kind of transparency to use.
      int RedT, GreenT,BlueT;             // Transparency RGB values.
      int AlphaT;                         // Transparency Alpha threshhold.
      TRANSPARENCYRAW trTransparency;     // How to handle transparency flag in raw output

      trTransparency = (ARG(PixelOut)) ? (TRANSPARENCYRAW) atoi (ARG(PixelOut)) : trNone0;
      ENSURE_(trMinex < trTransparency && trTransparency < trMaxex, "Pixel output type out of range.");
      
      ENSURE_(!(ARG(TransparentColor) && ARG(TransparentAlpha)), "Cannot use transparency by alpha and rgb at same time.");
      tkTransparency = tkNone;
      if (ARG(TransparentColor))
      {
         int result;
         result = sscanf (ARG(TransparentColor), "%d:%d:%d", &RedT, &GreenT, &BlueT);               
         qprintf(("Transparency Red=%d, G=%d, B=%d\n", RedT, GreenT, BlueT));               
         ENSURE_(3 == result, "Must provide all three components for r,g,b transprency. (no spaces)");
         ENSURE(0 <= RedT &&  RedT <=255);
         ENSURE(0 <= GreenT &&  GreenT <=255);
         ENSURE(0 <= BlueT &&  BlueT <=255);
         tkTransparency = tkRGB;
      }
      else if (ARG(TransparentAlpha))
      {
         AlphaT = atoi (ARG(TransparentAlpha));
         tkTransparency = (AlphaT < 0) ? tkAlphaHigh : tkAlphaLow;
         AlphaT = UTL_ABS (AlphaT);
      }
      
      dmDither = (ARG(Dither)) ? (DITHERMETHOD) (atoi (ARG(Dither))) : dmNone;
      if (!(dmMinex < dmDither && dmDither < dmMaxex))
      {
         EL_printf ("Error: Dither type is out of range.\n");
         RETURN EXIT_FAILURE;
      }
   
      pgff = ReadGFF (ARG(InFile));
      if (!pgff) 
      {
         EL_printf ("ERROR: Unable to read file '%s' as GFF file.\n", ARG(InFile));
         RETURN EXIT_FAILURE;
      }
      else
      {
         CHUNKNODE *pchunknodeRGBA;
         CHUNKNODE *pchunknodeRGBANew;
         CHUNKRGBA *pchunkrgbaNew;
         RGBADATA *prgbadataOld;
         RGBADATA *prgbadataNew;
         int Width, Height;
         
         /* Get RGBA node data for this image */
         pchunknodeRGBA = PChunkNodeOfId (pgff, IDRGBA);
         Width = pgff->pchunkggff->Data.Width;
         Height = pgff->pchunkggff->Data.Height;
         
         if (!pchunknodeRGBA)
         {
            EL_printf ("ERROR: GFF File '%s' does not have 32 bit RGBA data.\n");
            RETURN EXIT_FAILURE;
         }
         else
         {
            CHUNKRGBA *pchunkrgba;
            pchunkrgba = (CHUNKRGBA *)pchunknodeRGBA->pchunk;                                
            prgbadataOld = &pchunkrgba->Data;
         }
         
         
         /* Make new chunk for new image */
         pchunknodeRGBANew = CreateGFFChunkNodeNoFail (IDRGBA, 
            (sizeof(RGBADATA) * Width * Height));
         pchunkrgbaNew = (CHUNKRGBA *)pchunknodeRGBANew->pchunk;
         prgbadataNew = &pchunkrgbaNew->Data;


         DetermineTransparency (prgbadataNew, prgbadataOld, Width, Height,
                  tkTransparency, AlphaT, RedT, GreenT, BlueT);
         

         switch (dmDither) {
         case dmNone:
            ReduceWithNoDither (prgbadataNew, prgbadataOld, Width, Height);
            break;
         case dmErrorPropagation:
            ReduceWithErrorPropagationDither (prgbadataNew, prgbadataOld, Width, Height);
            break;
         case dmOrdered:
            ReduceWithOrderedDither (prgbadataNew, prgbadataOld, Width, Height);
            break;
         default:
            ENSURE_ (FALSE, "Illegal Dither method value\n");
            break;
         }
         
         LST_Remove (pchunknodeRGBA);
         DestroyGFFChunkNode (pchunknodeRGBA);
         LST_AddTail (pgff->plistChunkNodes, pchunknodeRGBANew);
         
         if (ARG(Raw))
         {
            WriteRaw16(prgbadataNew, ARG(OutFile), Width, Height, trTransparency);  
         }
         else
         {
            WriteGFF (ARG(OutFile), pgff);      
         }
      }
   }
   RETURN EXIT_SUCCESS;
} 
ENDFUNCMAIN(main)


/*************************************************************************
                               WriteRaw16                                
 *************************************************************************

   SYNOPSIS
		void WriteRaw16 (
         RGBADATA *pgrgbadata, 
         char *pszFile, 
         int Width, 
         int Height,
         TRANSPARENCYRAW tr
      )

   PURPOSE
  		To write out 16bit raw file.
  
   INPUT
		prgbadata :
		pszFile   :
      Width     :
      Height    :
      tr        : How to handle transparency in output.
        
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   SEE ALSO
  
  
   HISTORY
		11/01/96 : Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void WriteRaw16 (
   RGBADATA *prgbadata, 
   char *pszFile, 
   int Width, 
   int Height,
   TRANSPARENCYRAW tr
)
BEGINPROC (WriteRaw16)
{
	int	fh;
   UINT32   sizeofPixelData;
   UINT16   *pu16Buffer;
   UINT16   *pu16;
   long i;
   
   /* Create 16 bit buffer */
   sizeofPixelData = (UINT32)Width * (UINT32)Height * sizeof(UINT16);
   MEM_AllocMemNoFail(pu16Buffer, sizeofPixelData);
   for (i = Width * Height, pu16 = pu16Buffer; 
      i; 
      i--, prgbadata++, pu16++
   )
   {
      *pu16 =   ( ((UINT16)(prgbadata->Red & 0xF8))  << 7)|
                  ( ((UINT16)(prgbadata->Green & 0xF8)) << 2)|
                  ( ((UINT16)(prgbadata->Blue & 0xF8))  >> 3);
      switch (tr) {
      case trNone0:
         break;
      case trNone1:
         *pu16 |= 0x8000;
         break;
      case trBlackAsBlue0:
         if (0 == prgbadata->Alpha)
         {// Transparent
            *pu16 = 0x0000;
         }
         else if (0 == *pu16)
         {
            *pu16 = 0x0001; // set blue to 1.  
         }
         break;
      case trBlackAsBlue1:
         if (0 == prgbadata->Alpha)
         {// Transparent
            *pu16 = 0x0000;
         }
         else if (0 == *pu16)
         {
            *pu16 = 0x0001; // set blue to 1.  
         }
         *pu16 |= 0x8000;
         break;
      case trHighBit0:
         if (0 == prgbadata->Alpha)
         {// Transparent
            *pu16 &= 0x7FFF; // clear high bit
         }
         break;
      case trHighBit1:
         if (0 == prgbadata->Alpha)
         {// Transparent
            *pu16 |= 0x8000; // set high bit
         }
         break;
      case trTrickHighBit0:
         if (0 == prgbadata->Alpha)
         {// Transparent
            *pu16 = 0x0000; 
         }
         #if 0
         else if (0xFFFF != prgbadata->Alpha)
         { // Trick 
            *pu16 &= 0x7FFF; 
            if (0x0000 == *pu16) { *pu16 = 0x0001; } // black to blue
         }
         #endif
         else
         { // Opaque
            *pu16 |= 0x8000;
         }
         break;
      case trTrickHighBit1:
         if (0 == prgbadata->Alpha)
         {// Transparent
            *pu16 = 0x8000; 
         }
         #if 0
         else if (0xFFFF != prgbadata->Alpha)
         { // Trick 
            *pu16 |= 0x8000; 
            if (0x8000 == *pu16) { *pu16 = 0x8001; } // black to blue
         }
         #endif
         else
         { // Opaque
            *pu16 &= 0x7FFF;
         }
         break;
      default:
         ENSURE(FALSE);
         break;
      }
                  
      if (ARG(LittleEndian))
      {
         *pu16 = NativeToLSBF16Bit(*pu16);
      }
      else
      {                  
         *pu16 = NativeToMSBF16Bit(*pu16);
      }
   }
   
   fh = CHK_WriteOpen (pszFile);
   CHK_Write (fh, pu16Buffer, sizeofPixelData);
   CHK_Close (fh);

   MEM_FreeMem (pu16Buffer);
       
} ENDPROC (WriteRaw16)
  

/*************************************************************************
                           ReduceWithNoDither                            
 *************************************************************************

   SYNOPSIS
		void ReduceWithNoDither (
		   RGBADATA *prgbadataNew, 
		   RGBADATA *prgbadataOld, 
		   int Width, 
		   int Height
		)

   PURPOSE
  		
  
   INPUT
		prgbadataNew :
		prgbadataOld :
		Width        :
		Height       :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   SEE ALSO
  
  
   HISTORY
		11/06/96 : Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void ReduceWithNoDither (
   RGBADATA *prgbadataNew, 
   RGBADATA *prgbadataOld, 
   int Width, 
   int Height
)
BEGINPROC (ReduceWithNoDither)
{
   int x, y;
   RGBADATA *prgbadataSrc, *prgbadataDst;
   
   for (y = Height, 
         prgbadataSrc = prgbadataOld,
         prgbadataDst = prgbadataNew;
         y; 
         y--
   )
   {
      for (x = Width; x; x--)
      {
      
         /* Copy value, round up lower 3 bits if won't overflow, then truncate */
         prgbadataDst->Red = prgbadataSrc->Red;
         if ((prgbadataDst->Red & 0xF8) < 0xF8) prgbadataDst->Red += 4;
         prgbadataDst->Red &= 0xF8;
         
         prgbadataDst->Green = prgbadataSrc->Green;
         if ((prgbadataDst->Green & 0xF8) < 0xF8) prgbadataDst->Green += 4;
         prgbadataDst->Green &= 0xF8;
         
         prgbadataDst->Blue = prgbadataSrc->Blue;
         if ((prgbadataDst->Blue & 0xF8) < 0xF8) prgbadataDst->Blue += 4;
         prgbadataDst->Blue &= 0xF8;
         
         ++prgbadataSrc;
         ++prgbadataDst;
      }
   }
   
} ENDPROC (ReduceWithNoDither)

/*************************************************************************
                     ReduceWithErrorPropagationDither                     
 *************************************************************************

   SYNOPSIS
		void ReduceWithErrorPropagationDither (
		   RGBADATA *prgbadataNew, 
		   RGBADATA *prgbadataOld, 
		   int Width, 
		   int Height
		)

   PURPOSE
  		
  
   INPUT
		prgbadataNew :
		prgbadataOld :
		Width        :
		Height       :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   SEE ALSO
  
  
   HISTORY
		11/06/96 : Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// Pixel error tracking struct.
typedef struct {
   float Red;
   float Green;
   float Blue;
} ERRPIX; 

void ReduceWithErrorPropagationDither (
   RGBADATA *prgbadataNew, 
   RGBADATA *prgbadataOld, 
   int Width, 
   int Height
)
BEGINPROC (ReduceWithErrorPropagationDither)
{
#define NUM_COLOR_COMPONENTS_PER_PIXEL 3
#define ERRTYPE   INT32
#define ErrValOfU8(v)   (INT32)((UINT32)(v) << 16)
#define U8OfErrVal(v)   (UINT8) ( ((UINT32)(v) >> 16) | ((v&0x80000000) ? 0xFFFF0000 : 0) )
#define ERR_4_POINT_0   (0x040000)

   int x, y;
   UINT8 *pu8ComponentSrc, *pu8ComponentDst;
   ERRTYPE *prErr, *prErrNextRow; // fixed point 8.8
   ERRTYPE *prErrBuff;             // Buffer for tracking error values for each color component.
   UINT32 NumPixels;
   UINT32 NumErrs;
   UINT32 SizeOfErrBuff;
   
   /*
   ** Allocate buffer for tracking error values of the color components 
   */
   NumPixels = ((UINT32)Width * (UINT32)Height);
   NumErrs = NumPixels * NUM_COLOR_COMPONENTS_PER_PIXEL;
   SizeOfErrBuff =  NumErrs * sizeof (ERRTYPE);
   MEM_CallocMemNoFail(prErrBuff, SizeOfErrBuff);
   
   /* Process the pixels */
   for ( 
      y = Height, 
         prErr = prErrBuff,
         prErrNextRow = prErrBuff + (Width * NUM_COLOR_COMPONENTS_PER_PIXEL),
         pu8ComponentSrc = &prgbadataOld->Red,
         pu8ComponentDst = &prgbadataNew->Red;
      y; 
      y--
   )
   {
      for (x = Width; x; x--)
      {
         int c; // component index 1..3
         
         // Is it a Transparent pixels
         if (0 == pu8ComponentDst[3])
         { // Then skip it.
            pu8ComponentSrc += 4;
            pu8ComponentDst += 4;
            prErr += NUM_COLOR_COMPONENTS_PER_PIXEL;
            prErrNextRow +=NUM_COLOR_COMPONENTS_PER_PIXEL;
            continue;
         }
         
         for (
            c = NUM_COLOR_COMPONENTS_PER_PIXEL; 
            c; 
            c--,
               pu8ComponentSrc++,
               pu8ComponentDst++,
               prErr++,
               prErrNextRow++
         )
         {
            ERRTYPE rValue;
            UINT8 u8Value;
            
            rValue = ErrValOfU8(*pu8ComponentSrc) + *prErr;
            if (rValue < ErrValOfU8(0))
            {
               u8Value = 0;
            }
            else if (rValue > ErrValOfU8(0xF8))
            {
               u8Value = 0xF8;
            }
            else
            {
               /* Round up to highest 5 bits */
               rValue = (rValue + ERR_4_POINT_0);  // add 4.5 to round up to high 5 bits.
               u8Value = U8OfErrVal(rValue);
            }
            *pu8ComponentDst = u8Value & 0xF8; // Take only the high five bits
         
            /*
            ** Calculate and distribute the error to neighboring 4 pixels that 
            ** have not already been processed.  See diagram below of relevant 
            ** 3x2 pixel area.  
            */
            //                x = Already processed pixel.x
            //       +-+-+-+  * = Current pixel being processed.
            //       |x|*|1|  1 = Unprocessed pixel which get 7/16 of error.
            //       +-+-+-+  2 = Unprocessed pixel which get 3/16 of error.
            //       |2|3|4|  3 = Unprocessed pixel which get 5/16 of error.
            //       +-+-+-+  4 = Unprocessed pixel which get 1/16 of error.
            //                # = Not part of image. Off edge. (This symbol used below). 
            {
               ERRTYPE rError;
               int icase;
               
               rError = ErrValOfU8((int)*pu8ComponentSrc - (int)*pu8ComponentDst);
               
               // Case statement makes sure not to assign values off bottom, left or right edges of image.
               //      Right Edge?     Left Edge?          Bottom Edge?
               icase = ((1==x) << 2) | ((Width==x) << 1) | (1==y);
               switch (icase) { 
               case 0: // Current pixel not on any edge of image
                  prErr[NUM_COLOR_COMPONENTS_PER_PIXEL]         += rError * 7/16;   //       +-+-+-+
                  prErrNextRow[-NUM_COLOR_COMPONENTS_PER_PIXEL] += rError * 3/16;   //       |x|*|1|
                  prErrNextRow[0]                               += rError * 5/16;   //       +-+-+-+
                  prErrNextRow[NUM_COLOR_COMPONENTS_PER_PIXEL]  += rError * 1/16;   //       |2|3|4|
                  break;                                                            //       +-+-+-+
               case 1: // Current pixel on bottom edge of image.
                  prErr[NUM_COLOR_COMPONENTS_PER_PIXEL]         += rError * 7/16;   //       +-+-+-+
                  //prErrNextRow[-NUM_COLOR_COMPONENTS_PER_PIXEL] += rError * 3/16; //       |x|*|1|
                  //prErrNextRow[0]                               += rError * 5/16; //       +-+-+-+
                  //prErrNextRow[NUM_COLOR_COMPONENTS_PER_PIXEL]  += rError * 1/16; //       |#|#|#|
                  break;                                                            //       +-+-+-+
               case 2: // Current pixel on left edge of image.
                  prErr[NUM_COLOR_COMPONENTS_PER_PIXEL]         += rError * 7/16;   //       +-+-+-+
                  //prErrNextRow[-NUM_COLOR_COMPONENTS_PER_PIXEL] += rError * 3/16; //       |#|*|1| 
                  prErrNextRow[0]                               += rError * 5/16;   //       +-+-+-+ 
                  prErrNextRow[NUM_COLOR_COMPONENTS_PER_PIXEL]  += rError * 1/16;   //       |#|3|4| 
                  break;                                                            //       +-+-+-+ 
               case 3: // Current pixel on bottom left corner of image.
                  prErr[NUM_COLOR_COMPONENTS_PER_PIXEL]         += rError * 7/16;   //       +-+-+-+
                  //prErrNextRow[-NUM_COLOR_COMPONENTS_PER_PIXEL] += rError * 3/16; //       |#|*|1| 
                  //prErrNextRow[0]                               += rError * 5/16; //       +-+-+-+ 
                  //prErrNextRow[NUM_COLOR_COMPONENTS_PER_PIXEL]  += rError * 1/16; //       |#|#|#| 
                  break;                                                            //       +-+-+-+ 
               case 4: // Current pixel on right edge of image.
                  //prErr[NUM_COLOR_COMPONENTS_PER_PIXEL]         += rError * 7/16; //       +-+-+-+
                  prErrNextRow[-NUM_COLOR_COMPONENTS_PER_PIXEL] += rError * 3/16;   //       |x|*|#| 
                  prErrNextRow[0]                               += rError * 5/16;   //       +-+-+-+ 
                  //prErrNextRow[NUM_COLOR_COMPONENTS_PER_PIXEL]  += rError * 1/16; //       |2|3|#| 
                  break;                                                            //       +-+-+-+ 
               case 5: // Current pixel on bottom right corner of image.   
                  //prErr[NUM_COLOR_COMPONENTS_PER_PIXEL]         += rError * 7/16; //       +-+-+-+
                  //prErrNextRow[-NUM_COLOR_COMPONENTS_PER_PIXEL] += rError * 3/16; //       |x|*|#| 
                  //prErrNextRow[0]                               += rError * 5/16; //       +-+-+-+ 
                  //prErrNextRow[NUM_COLOR_COMPONENTS_PER_PIXEL]  += rError * 1/16; //       |#|#|#| 
                  break;                                                            //       +-+-+-+ 
               case 6:
                  ENSURE(FALSE); // impossible case. x cannot be 1 and Width at same time.
                  break;
               case 7:
                  ENSURE(FALSE); // impossible case. x cannot be 1 and Width at same time.
                  break;
               }
            }
         } // End Component loop
         
         /* skip over the alpha component */
         ++pu8ComponentDst;
         ++pu8ComponentSrc;
         
      } // End x loop
      
   } // End y loop
   
   MEM_FreeMem (prErrBuff);

} ENDPROC (ReduceWithErrorPropagationDither)

/*************************************************************************
                         ReduceWithOrderedDither                         
 *************************************************************************

   SYNOPSIS
		void ReduceWithOrderedDither (
		   RGBADATA *prgbadataNew, 
		   RGBADATA *prgbadataOld, 
		   int Width, 
		   int Height
		)

   PURPOSE
  		
  
   INPUT
		prgbadataNew :
		prgbadataOld :
		Width        :
		Height       :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   SEE ALSO
  
  
   HISTORY
		11/06/96 : Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void ReduceWithOrderedDither (
   RGBADATA *prgbadataNew, 
   RGBADATA *prgbadataOld, 
   int Width, 
   int Height
)
BEGINPROC (ReduceWithOrderedDither)
{
   int x, y;
   UINT8 c = 0;
   RGBADATA *prgbadataSrc, *prgbadataDst;
   
   for (y = Height, 
         prgbadataSrc = prgbadataOld,
         prgbadataDst = prgbadataNew;
         y; 
         y--
   )
   {
      for (x = Width; x; x--)
      {
      
         #if 0
         /* Copy value, round up lower 3 bits if won't overflow, then truncate */
         prgbadataDst->Red = prgbadataSrc->Red;
         if ((prgbadataDst->Red & 0xF8) < 0xF8) prgbadataDst->Red += 4;
         prgbadataDst->Red &= 0xF8;
         
         prgbadataDst->Green = prgbadataSrc->Green;
         if ((prgbadataDst->Green & 0xF8) < 0xF8) prgbadataDst->Green += 4;
         prgbadataDst->Green &= 0xF8;
         
         prgbadataDst->Blue = prgbadataSrc->Blue;
         if ((prgbadataDst->Blue & 0xF8) < 0xF8) prgbadataDst->Blue += 4;
         prgbadataDst->Blue &= 0xF8;
         
         #else
         /* Copy value, round up lower 3 bits if won't overflow, then truncate */
         prgbadataDst->Red = c;
         prgbadataDst->Green = c;
         prgbadataDst->Blue = c;
         prgbadataDst->Alpha = prgbadataSrc->Alpha;
         #endif
                           
         ++prgbadataSrc;
         ++prgbadataDst;
      }
         ++c;
   }


} ENDPROC (ReduceWithOrderedDither)
 
/*************************************************************************
                          DetermineTransparency                          
 *************************************************************************

   SYNOPSIS
		void DetermineTransparency (
		   RGBADATA *prgbadataNew, 
		   RGBADATA *prgbadataOld, 
		   int Width, 
		   int Height,
		   TRANSPARENCYKIND tk,
		   UINT8 Alpha,
		   UINT8 Red,
		   UINT8 Green,
		   UINT8 Blue
		)

   PURPOSE
  		Set the alpha values in the new rgb chunk to 0 for transparent pixels
      and 255 for opaque pixels, based on transparency criteria.
  
   INPUT
		prgbadataNew :
		prgbadataOld :
		Width        :
		Height       :
		tk           :
		Alpha        :
		Red          :
		Green        :
		Blue         :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   SEE ALSO
  
  
   HISTORY
		11/07/96 : Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void DetermineTransparency (
   RGBADATA *prgbadataNew, 
   RGBADATA *prgbadataOld, 
   int Width, 
   int Height,
   TRANSPARENCYKIND tk,
   UINT8 Alpha,
   UINT8 Red,
   UINT8 Green,
   UINT8 Blue
)
BEGINPROC (DetermineTransparency)
{
   int x, y;
   UINT8 c = 0;
   RGBADATA *prgbadataSrc, *prgbadataDst;
   
   for (
      y = Height, 
         prgbadataSrc = prgbadataOld,
         prgbadataDst = prgbadataNew;
      y; 
      y--
   )
   {
      for (
         x = Width; 
         x; 
         x--,
         prgbadataSrc++,
         prgbadataDst++
            
      )
      {
      
         prgbadataDst->Alpha = 255; // assume opaque
         switch (tk) {
         case tkNone:
            break;
         case tkAlphaLow:
            if (prgbadataSrc->Alpha <= Alpha)
            {
               prgbadataDst->Alpha = 0;
            }
            break;
         case tkAlphaHigh:
            if (prgbadataSrc->Alpha >= Alpha) 
            {
               prgbadataDst->Alpha = 0;
            }
            break;
         case tkRGB:
            if (prgbadataSrc->Red == Red && prgbadataSrc->Green == Green && prgbadataSrc->Blue == Blue) 
            {
               prgbadataDst->Alpha = 0;
            }
            break;
         }
      }
   }

} ENDPROC (DetermineTransparency)
 
