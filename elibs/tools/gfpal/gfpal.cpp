/*************************************************************************
 *                                                                       *
 *                              GFPAL.CPP                                *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION
   
      Takes image file(s) and an optional input palette and creates a palette 
      or a palettized image output file.
      

   PROGRAMMERS
      Juan M. Alvarado

   FUNCTIONS

   TABS : 4 7

   HISTORY
		08/01/96 : JMA Created.
		02/28/97 : JMA Made it only ouput one file at a time: image or palette. 
                  Simplified switches interface. Added image output mode that
                  has 32 bit header on raw data that tells about transparency.
		02/28/97 : JMA Added inverse palette saving with GFF palette to speed things
                  up when you supply the palette for use without changes.
                  
            
   TODO
      O Save inverse palette with GFF palette so you don't have to build it again if that palette
         is passed in for use as is.
      O Implement dithering: Ordered dither (preferable for low resolution displays like psx)
         Algorithms in Graphics Gems II page 72.
         Also discusses gamma correction in that same gem.
         Other applicable Gems:
            Image smoothing and sharpening by discrete convolution:
               Graphics Gems II p. 50
            
               
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
#include <echidna\dbmess.h>

#include "quantize.h"

#if 0
	#define qprintf(args)	EL_printf args
#else
	#define qprintf(args)
#endif

/*************************** C O N S T A N T S ***************************/

#define ENTRIES_MAX  256
#define ENTRY_INDEX_MAX  (ENTRIES_MAX - 1)

#define CODEDRAW_ALL_TRANSPARENT     (1 << 24)
#define CODEDRAW_SOME_TRANSPARENT    (1 << 25)    

/******************************* T Y P E S *******************************/

typedef enum {
   dmMinex = -1,
   dmNone,
   dmErrorPropagation,
  // dmOrdered,
   dmMaxex
} DITHERMETHOD;

typedef enum {
   histmergeSum,
   histmergeMax
} HISTMERGE;

typedef enum {
   tkNone,     // No transparency in this image
   tkAlphaLow,  // Transparent if Alpha below given threshhold
   tkAlphaHigh,  // Transparent if Alpha above given threshhold
   tkRGB       // Transparent if Red, Green and Blue component equal given values.
} TRANSPARENCYKIND;

typedef enum {
   ioMinex = -2,
   ioNone,  // no raw output.
   ioLRTB,  // raw output left right top to bottom
   ioGFF,   // GFF file output
   ioCLRTB, // Coded raw. Just like ioLRTB but with 32 bit header of flags.
   ioSony4BitCLRTB, // Coded raw. Just like ioLRTB but with 32 bit header of flags.
   ioCDLRTB, // Coded raw with dimensions. Just like ioLRTB but with 32 bit header of flags.
   ioMaxex
} IMGOUT;

typedef enum {
   kpMinex = -2,
   kpNone,        // no palette file output.
   kpRGB,         // raw output RGB format.
   kpGFF,         // GFF format.
   kpDirectX,     // raw output left right top to bottom
   kpSony16,      // 16 entry palette.
   kpKludge,
   kpMaxex
} KINDPAL;

/************************** P R O T O T Y P E S **************************/

BOOL BuildHistogramForFile (
   char *pszFileName,
   HIST_ENTRY_TYPE *pHistogram,
   TRANSPARENCYKIND tk,
   UINT8 Alpha,
   UINT8 Red,
   UINT8 Green,
   UINT8 Blue
);
BOOL MergeHistograms ( HIST_ENTRY_TYPE *pHistogram, HIST_ENTRY_TYPE *pHistogramCrnt);

BOOL PalettizeImageFile (
   char *pszFileName,
   char *pszOutFileName,
   UINT8 *inv_cmap,
   UINT8 *pColorMap,
   int NumColors,
   TRANSPARENCYKIND tk,
   UINT8 Alpha,
   UINT8 Red,
   UINT8 Green,
   UINT8 Blue,
   UINT8 IndexT,
   int   *pNumUsed,
   IMGOUT imgout,
   DITHERMETHOD dmDither,
   PALETTE_SITE *arpalsite 
);
BOOL ParseRangeList (
   LST_LIST *plist, 
   PALETTE_SITE *arpalsite, 
   SITEKIND SiteKind, 
   int *pNumSet,
   int *pEntryHighest
   );

BOOL ParseRange (
   char *psz, 
   PALETTE_SITE *arpalsite, 
   SITEKIND SiteKind, 
   int *pNumSet,
   int *pEntryHighest
   );

UINT8 *ReadAPalette (
   char *pszPaletteFile,
   int *pNumInPalette,
   UINT8 **ppinvcmap
);

BOOL WritePaletteFile (
   char *pszOutFile, 
   uint8 *pColorMap, 
   int TotalColors, 
   UINT8 *pinvcmap,
   PALETTE_SITE *arpalsite,
   TRANSPARENCYKIND tk,
   UINT8 IndexT
   );
BOOL WriteAnyPaletteFile (
   char *pszOutFile, 
   uint8 *pColorMap, 
   int TotalColors, 
   KINDPAL kindpal, 
   UINT8 *pinvcmap,
   PALETTE_SITE *arpalsite, 
   TRANSPARENCYKIND tk,
   UINT8 IndexT
   );

/***************************** G L O B A L S *****************************/

HISTMERGE HistMergeMethod;
BOOL  fBigEndian;
int   fQuiet;

/****************************** M A C R O S ******************************/

#define round(a)  floor((a) + 0.5)


/**************************** R O U T I N E S ****************************/


/*************************** ArgParse Template ***************************/
enum {
   NDX_Dither,
   NDX_ImageOut,
   NDX_BigEndian,
   NDX_OutPalKind,
   NDX_MaxNewColors,
   NDX_PaletteDefault,
   NDX_PaletteOutFile,
   NDX_UsableIndexes,
   NDX_BlockedIndexes,
   NDX_ConstantIndexes,
   NDX_SumHistograms,
   NDX_TransparentColor,
   NDX_TransparentAlpha,
   NDX_TransparentIndex,
   NDX_Quiet,
   NDX_OutFile,
   NDX_InFileList,
};
#define ARG(name) (newargs [NDX_ ## name])

static char Usage[] = "Usage: GfPal [Switches] OUTFILE INFILES\n";
static char	**newargs;
   
// Guide or keeping help to 80 columns:   
//    "         1         2         3         4         5         6         7         8"
//    "12345678901234567890123456789012345678901234567890123456789012345678901234567890"
ArgSpec Template[] = {
   {CHRKEYWORD_ARG,              "D",	   
      "    -D<method>     Dithering method to use:.\n"
      "                      0 = None (default).\n"
      "                      1 = Error Propogation (best for hi-res image).\n"
   ,},
   {CHRKEYWORD_ARG, "R",
      "    -R<code>       Image output format for OUTFILE.\n"
      "                      code Description\n"
      "                      ---- -----------\n"
      "                       0   Raw:  pixel dump left to right, top to bottom.\n"
      "                       1   GFF: (Default)\n"
      "                       2   Coded Raw: = Raw with 32 bit header word:\n"
      "                              Bit(s)   Meaning (regarding image data)\n"
      "                              ------   -------\n"
      "                                24     All transparent (no pixels written).\n"
      "                                25     At least one transparent pixel.\n"
      "                       3   Coded Raw Sony: = 4 bit raw with 32 bit header\n"
      "                       4   Coded Raw Dimensioned: = Like Coded Raw but with\n"
      "                              16 bit Width and Height after 32 bit header.\n"
   ,},
   {SWITCH_ARG,              "-MSBF=MSBF",	   
      "    -MSBF          Write big endian data words. Default is little endian.\n"
   ,},
   {CHRKEYWORD_ARG, "K",
      "    -K<code>       Write output palette file of type <mode>: (See -X):\n"
      "                      code Description\n"
      "                      ---- -----------\n"
      "                       0   Raw RGB: 3 bytes per entry, 1 byte per compnent\n"
      "                       1   GFF file.\n"
      "                       2   Direct X palette.\n"
      "                       3   Sony 16 color palette.\n"
   ,},
   {CHRKEYWORD_ARG, "M",	      
      "    -M<new colors> Number of new colors to create maximum (default=256).\n"
      "                      Alternative to -U. Uses first unblocked (-B) and\n"
      "                      non-constant (-C) indices.\n"
   ,},
   {CHRKEYWORD_ARG, "P",
      "    -P<file>       Palette file get colors from: *.pcx or *.gff or *.raw.\n"
   ,},
   {CHRKEYWORD_ARG, "O",
      "    -O<file>       Ouput file for palette if outputting image and palette.\n"
   ,},
   {CHRKEYWORD_ARG|MULTI_ARG|LIST_ARG, "U",
      "    -U<ranges>     Use only these palette indices for new colors.\n"
      "                      Alternative to -B and -M. Assumes all other indices\n"
      "                      blocked unless marked as constant with -C\n"
      //"                      Where <ranges>:=<index>|<range> <ranges>\n"
      //"                            <range> :=<index>..<index>\n"
      //"                      Where <index> is a number from 0 through 255.\n"
      "                      Example use on command line:\n"
      "                         -u5 9 16..32 -u 128..271 240..255\n"
   ,},
   {CHRKEYWORD_ARG|MULTI_ARG|LIST_ARG, "B",
      "    -B<ranges>     Block palette indices from being changed or referenced.\n"
      "                      Alternative to -U. Assumes all other indices settable\n"
      "                      unless marked as constant with -C\n"
      "                      Where <ranges> is defined as in -U above.\n"
   ,},
   {CHRKEYWORD_ARG|MULTI_ARG|LIST_ARG, "C",
      "    -C<ranges>     Constant palette indices have colors that may be\n"
      "                      referenced but not changed.\n"
      "                      Where <ranges> is defined as in -U above.\n"
   ,},
   {CHRSWITCH_ARG, "S",
      "    -S             Sum histograms. Use sum (vs. max) of histogram entries\n"
      "                      across multiple frames.\n"
      //"                      Sum good for collage frames.\n"
      //"                      Max good for animation frames.\n"
   ,},
   {KEYWORD_ARG, "-TC=TC",	      
      "    TC<r:g:b>      Transparency by r,g,b value\n"
   , },
   {KEYWORD_ARG, "TA",	      
      "    -TA<alpha>     Transparency by alpha value.\n"
      "                      If alhpa >= 0 then transparent values <= alpha.\n"
      "                      If alhpa <  0 then transparent values >= -alpha.\n"
   ,},      
   {KEYWORD_ARG, "-TI=TI",	      
      "    -TI<Index>     Palette Index to set transparent pixels to. Defaule=0.\n"
      "                      This index is automatically blocked (-B).\n"
   ,},      
   {CHRSWITCH_ARG, "Q",        
      "    -Q             Quiet. No progress printing.\n"
   ,},
   {STANDARD_ARG|REQUIRED_ARG, "OUTFILE",	
      "    OUTFILE        Output file.\n" 
   ,},
   {STANDARD_ARG|MULTI_ARG|LIST_ARG,  "INFILES",	
      "    INFILES        GFF file(s) to read. May list multiple files for\n"
      "                      generating palette suitable for use with all\n"
      "                      listed files.\n"
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
      UINT8 *pColorMap;                   // Actual output palette for image. May have colors not used by image.
      PALETTE_SITE *arpalsite;            // Array that describes the usage status of each palette entry.
      
      int EntriesTotal;                   // Number of entries in final palette.
      int EntriesChangeableMax;           // Max new colors quantizer may create (may be 0 if just mapping to set palette)
      int EntriesBlocked;                 // Num entries in palette not usable.
      int EntriesSeeded;                  // Num entries in default palette that are usable but not changeable.
      int EntriesUsable;                  // Num entries resultant from quantization process == num usable by inverse color mapper.
                                          //    EntriesUsable <= (EntriesChangeableMax + EntriesSeeded).  
                                          //    Will be less if image requires less colors than user allowed.
      int EntriesUsed;                    // Actual number of entries mapped to (not including tranparent index).
      
      int TransparentIndex;               // Index to set transparent pixels to.
      TRANSPARENCYKIND tkTransparency;    // What kind of transparency to use.
      int RedT, GreenT,BlueT;             // Transparency RGB values.
      int AlphaT;                         // Transparency Alpha threshhold.
      IMGOUT   imgout;                    // What kind of raw output to write. roNone means write GFF image file.
      KINDPAL  kindpal;                   // What kind of output palette file to write. 
      DITHERMETHOD   dmDither;      
      UINT8 *pinvcmap;                    // Pointer to inverse color map.
      BOOL  fImagesAllTransparent;        // True if no colors to process from image because images are completely transparent.
      
      pinvcmap = NULL;
      fImagesAllTransparent = FALSE;
      
      fQuiet = ARG(Quiet) != NULL;
      fBigEndian = ARG(BigEndian) != NULL;
      
      if (!ARG(InFileList) && !( ARG(PaletteDefault) && ARG(OutPalKind) ) )
      {
         EL_printf ("Error: INFILES required unless you are using -p with -k to convert a palette.\n");
         RETURN EXIT_FAILURE;
      }
      #if 0
      if (ARG(OutPalKind) && ARG(ImageOut))
      {
         EL_printf ("Error: Only one output allowed, image or palette (i.e. -R and -K are mutually exclusive.)\n");
         RETURN EXIT_FAILURE;
      }
      #endif
      
      dmDither = (ARG(Dither)) ? (DITHERMETHOD) (atoi (ARG(Dither))) : dmNone;
      if (!(dmMinex < dmDither && dmDither < dmMaxex))
      {
         EL_printf ("Error: Dither type is out of range.\n");
         RETURN EXIT_FAILURE;
      }
        
      imgout = (IMGOUT)(ARG(ImageOut) ? (atoi (ARG (ImageOut))) : ioGFF);
      ENSURE_((ioNone < imgout && imgout < ioMaxex), "Image output mode out of range");

      kindpal = kpNone;
      if (ARG (OutPalKind)) 
      {
         kindpal = (KINDPAL)atoi(ARG (OutPalKind));
         ENSURE_((kpNone < kindpal && kindpal < kpMaxex), "Palette output kind is out of range");
      }

      EntriesTotal = ENTRIES_MAX;            
      EntriesBlocked = 0;
      EntriesSeeded = 0;
      EntriesChangeableMax = ARG(MaxNewColors) ?  atoi(ARG(MaxNewColors)) : ENTRIES_MAX;
      ENSURE_((0 <= EntriesChangeableMax && EntriesChangeableMax <= ENTRIES_MAX), "Invalid number of colors specified.");

      HistMergeMethod = (ARG(SumHistograms)) ? histmergeSum : histmergeMax;

      ENSURE_(!(ARG(TransparentColor) && ARG(TransparentAlpha)), "Cannot use transparency by alpha and rgb at same time.");
      TransparentIndex = (ARG(TransparentIndex)) ? atoi (ARG(TransparentIndex)) : 0;
      ENSURE_(0 <= TransparentIndex && TransparentIndex <= ENTRY_INDEX_MAX, "Transparent Index out of range 0..255.");
      tkTransparency = tkNone;
      if (ARG(TransparentColor))
      {
         int result;
         result = sscanf (ARG(TransparentColor), "%d:%d:%d", &RedT, &GreenT, &BlueT);               
         qprintf(("Transparency Red=%d, G=%d, B=%d\n", RedT, GreenT, BlueT));               
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
 
      /*
      ** Allocate color map and palette usage array tracker
      */
      MEM_CallocMemNoFail (arpalsite, (ENTRIES_MAX * sizeof(PALETTE_SITE)));
      MEM_CallocMemNoFail (pColorMap, (ENTRIES_MAX * 3));
      
      /*
      ** Read in default palette if any 
      */
      if (ARG(PaletteDefault))
      {
         int NumInPalette;
         UINT8 *pu8;
         qprintf (("Reading palette from file %s\n", ARG(PaletteDefault)));
         pu8 = ReadAPalette (ARG(PaletteDefault), &NumInPalette, &pinvcmap); // ReadRawPalette (ARG(PaletteDefault), &NumInPalette);
         if (pu8)
         {
            int NumToCopy;
            int i;
            PALETTE_SITE *ppalsite;
            UINT8 *pu8Temp;
            UINT8 *pCMap;
            
            NumToCopy = UTL_MIN (NumInPalette, ENTRIES_MAX);
//            EntriesTotal = NumToCopy;
            qprintf (("Copying %d colors from palette in file %s\n", NumToCopy, ARG(PaletteDefault)));
            for (i = 0, ppalsite = arpalsite, pCMap = pColorMap, pu8Temp = pu8; i < NumToCopy; i++, ppalsite++, pu8Temp += 3)
            {
               *pCMap++ = ppalsite->r = pu8Temp[0];
               *pCMap++ = ppalsite->g = pu8Temp[1];
               *pCMap++ = ppalsite->b = pu8Temp[2];
            }
            FreeRawPalette (pu8);
            qprintf(("%d colors copied from palette file %s\n", NumToCopy, ARG(PaletteDefault)));
         }
         else
         {
            EL_printf ("Error: Could not load palette from file %s\n", ARG(PaletteDefault));
            RETURN EXIT_FAILURE;
         }
      }
      
      /*
      ** Read palette restrictions. 
      */
      if (ARG (ConstantIndexes))
      {
         int EntrySeededHighest;
         
         if (!ARG(PaletteDefault)) 
         {
            EL_printf ("Error: Must provide default palette when using constant palette indices (-C)\n");
            RETURN EXIT_FAILURE;
         }
         if (!ParseRangeList (MULTI_ARGLINKEDLIST (ARG (ConstantIndexes)), arpalsite, skSeeded, &EntriesSeeded, &EntrySeededHighest))
         {
            RETURN EXIT_FAILURE;
         }
      }
      
      if (ARG (BlockedIndexes))
      {
         int EntryBlockedHighest;
         if (ARG(UsableIndexes))
         {
            ErrMess("-B and -U are mutually exclusive alternatives.\n");
            RETURN EXIT_FAILURE;
         }
         if (!ParseRangeList (MULTI_ARGLINKEDLIST (ARG (BlockedIndexes)), arpalsite, skBlocked, &EntriesBlocked, &EntryBlockedHighest))
         {
            RETURN EXIT_FAILURE;
         }
      }     
      else if (ARG (UsableIndexes))
      {
         int EntryUsableHighest, i;
         
         if (ARG(MaxNewColors))
         {
            ErrMess("-M and -U are mutually exclusive alternatives.\n");
            RETURN EXIT_FAILURE;
         }
         if (!ParseRangeList (MULTI_ARGLINKEDLIST (ARG (UsableIndexes)), arpalsite, skToOpen, &EntriesChangeableMax, &EntryUsableHighest))
         {
            RETURN EXIT_FAILURE;
         }
         for (i = 0; i < ENTRIES_MAX; i++)
         {
            if (skOpen == arpalsite[i].SiteKind)
            {
               arpalsite[i].SiteKind = skBlocked;
            } 
            else if (skToOpen == arpalsite[i].SiteKind)
            {
               arpalsite[i].SiteKind = skOpen;
            }
         }
      }  
         
      if (ARG(MaxNewColors))
      {
         /*
         ** Compute open/blocked indices by setting first EntriesChangeableMax 
         ** open indices to open and the remaining open indices to blocked 
         */
         int Usable, i;
         
         Usable = 0;
         for (i = 0; i < ENTRIES_MAX; i++)
         {
            if (skOpen == arpalsite[i].SiteKind)
            {
               if (Usable < EntriesChangeableMax) 
               {
                  ++Usable; // leave it open and count it.
               }
               else
               {
                  arpalsite[i].SiteKind = skBlocked; // got enough open, block the rest.
               }
            } 
         }
         if (Usable < EntriesChangeableMax)
         {
            ErrMess ("Not enough unblocked indices to make %d colors\n", EntriesChangeableMax);
            RETURN EXIT_FAILURE;
         }
      }
      /* Find number of entries changeable */
      {
         int  i;
         EntriesChangeableMax = 0;
         for (i = 0; i < ENTRIES_MAX; i++)
         {
            if (skOpen == arpalsite[i].SiteKind)
            {
               ++EntriesChangeableMax;
            } 
         }
      }
          
      /*
      ** If just translating a palette or using default palette but no entries 
      ** have been blocked or seeded then make all entries seeded.
      */
      if ( (ARG(OutPalKind)  && ARG(PaletteDefault)) || 
         (ARG (PaletteDefault) && 0 == EntriesBlocked && 0 == EntriesSeeded)
      )
      {
         char szRange[64];
         int EntrySeededHighest;
         
         /* Assuming constant palette with all entries usable. */
         sprintf(szRange, "0..%d", EntriesTotal-1);                  
         EntriesChangeableMax = 0;
         EntriesBlocked = 0; 
         EntriesSeeded = 0; 
         EntrySeededHighest = 0;
         if (!ParseRange (szRange, arpalsite, skSeeded, &EntriesSeeded, &EntrySeededHighest))
         {
            RETURN EXIT_FAILURE;
         }
      }

      /*
      ** If transparency is being used or we are outputting just a palette that 
      ** has a transparency index, then make sure the transparency index is 
      ** blocke from use as a color 
      */
      if (tkNone != tkTransparency || (ARG(OutPalKind) && ARG(TransparentIndex)))
      { /* Has transparency, so make sure transparent index is blocked */
         if (arpalsite[TransparentIndex].SiteKind != skBlocked)
         {
            if (skSeeded == arpalsite[TransparentIndex].SiteKind)
            {
               --EntriesSeeded;
            }
            else if (skOpen == arpalsite[TransparentIndex].SiteKind)
            {
               --EntriesChangeableMax;
            }
            arpalsite[TransparentIndex].SiteKind = skBlocked;
            ++EntriesBlocked;
            qprintf (("Blocked Transparent index from use for color.\n"));
         }
      }
      
      /* Make sure there are enough entries in palette for MaxNewColors */
      {
         int temp;
         temp = EntriesBlocked + EntriesSeeded +  EntriesChangeableMax;
//         EntriesTotal = UTL_MAX(EntriesTotal, temp);         
         if (EntriesTotal > ENTRIES_MAX)
         {
            EL_printf ("Error: Total request for blocked (%d), constant (%d) and new color (%d) entries exceeds palette limit of %d\n", 
               EntriesBlocked, EntriesSeeded, EntriesChangeableMax, ENTRIES_MAX);
         }
         qprintf(("Palette will have %d entries.\n", EntriesTotal));
      }
      
      if (!EntriesChangeableMax && !EntriesSeeded)
      {
         EL_printf ("Error: No colors to use. Nothing to do.\n");
         RETURN EXIT_FAILURE;
      }
      
      if (ARG(InFileList) && EntriesChangeableMax)
      {
         HIST_ENTRY_TYPE *pHistogram;        // Histogram for all images.
         HIST_ENTRY_TYPE *pHistogramCrnt;    // Histogram for current image.
         
         // Allocate Histograms
         MEM_CallocMemNoFail (pHistogram, (HIST_CELLS * sizeof(HIST_ENTRY_TYPE)));
         MEM_AllocMemNoFail (pHistogramCrnt, (HIST_CELLS * sizeof(HIST_ENTRY_TYPE)));
         
         /*
         ** Build Histogram
         */
         {
            LST_LIST	*listInFiles;
            LST_NODE	*pnode;
            int NumInFiles;
         
            listInFiles = MULTI_ARGLINKEDLIST (ARG(InFileList));
            qprintf (("Building Histogram for:\n"));
            for (
               pnode = LST_Head (listInFiles),
                  NumInFiles = 0;
               !LST_IsEOList(pnode);
               pnode = LST_Next (pnode),
                  NumInFiles++
            )
            {
                qprintf (("   %s\n", LST_NodeName (pnode)));
                if (!BuildHistogramForFile (LST_NodeName(pnode), pHistogramCrnt, 
                  tkTransparency, AlphaT, RedT, GreenT, BlueT)
                )
                {
                  RETURN EXIT_FAILURE;
                }
                fImagesAllTransparent = !MergeHistograms (pHistogram, pHistogramCrnt);
            }
            if (fImagesAllTransparent)
            {
               qprintf (("Image(s) are completely transparent\n"));
            }
         }
         
         if (!fImagesAllTransparent)
         {
            /*
            ** Quantize colors
            */
            qprintf (("Quantizing to a maximum of %d new colors...\n", EntriesChangeableMax));
            {
               if (quantize(pHistogram, EntriesChangeableMax, pColorMap, &EntriesUsable,
                  arpalsite, ENTRIES_MAX
               ))
               {
                  EL_printf ("Error: unable to quantize\n");
                  RETURN EXIT_FAILURE;
               }
            }  
            qprintf (("Quantization yields maximum of %d usable colors\n", EntriesUsable));
            
         }
         
         // Free historgrams
         MEM_FreeMem (pHistogramCrnt);
         MEM_FreeMem (pHistogram);
      }
      else
      {
         EntriesUsable = EntriesSeeded;
      }

      /*
      ** Create Inverse Color Map if necessary (if dont already have one and image output of gff palette output) 
      */
      if (!fImagesAllTransparent &&
            (
               (!pinvcmap && (ARG(ImageOut) || kpGFF == kindpal)) // don't have one and need one for image or gff palette
               || (!ARG(OutPalKind) && EntriesChangeableMax)       // colors are being generated.
            )
      )   
      {
         /*
         ** Allocate Inverse Color Map.
         */
         MEM_CallocMemNoFail (pinvcmap,HIST_CELLS);

         /*
         ** Create the Inverse Color Map for the palette.  
         */
         {
            UINT8 *pColorsUsed;  // Temp palette of just the colors used by image. For inverse mapping routines.
            int   *pMapFromUsed;
         
            MEM_CallocMemNoFail (pColorsUsed, (ENTRIES_MAX * 3));
            MEM_CallocMemNoFail (pMapFromUsed, (ENTRIES_MAX * sizeof(int)));
         
            /*
            ** Build second palette that has all the usable colors at the start for 
            ** feeding to inverse color map routine.  At same time build map from 
            ** this condensed palette back to the real palette.  
            */
            {
               int i, j;;
               UINT8 *pMap;
               UINT8 *pUsed;
               PALETTE_SITE *ppalsite;
               
               j = 0; 
               pUsed = pColorsUsed;
               for (i = 0, pMap = pColorMap, ppalsite = arpalsite; 
                  (i < EntriesTotal && j < EntriesUsable); 
                  i++, pMap += 3, ppalsite++)
               {
                  if (skOpen == ppalsite->SiteKind || skSeeded == ppalsite->SiteKind)
                  {
                     *pUsed++ = pMap[0];
                     *pUsed++ = pMap[1];
                     *pUsed++ = pMap[2];
                     pMapFromUsed[j] = i;
                     j++;
                  }
               }
               ENSURE (j == (EntriesUsable));
            }
            
            /*
            ** Build Inverse Color Map for the temp palette.  (all this could 
            ** be hid away in wrapper around call to inv_cmap_2) 
            */
            {
               UINT8 *cmap[3];
               unsigned long *dist_buf;
               
               // Allocate parallel arrays of the R G B values of the color map for.
               MEM_CallocMemNoFail (cmap[0],ENTRIES_MAX);
               MEM_CallocMemNoFail (cmap[1],ENTRIES_MAX);
               MEM_CallocMemNoFail (cmap[2],ENTRIES_MAX);
               
               qprintf (("Building inverse colormap...\n"));
               {
                  int i;
                  for (i = 0; i < EntriesUsable; i++)
                  {
                     cmap[0][i] = pColorsUsed[i * 3 + 0];
                     cmap[1][i] = pColorsUsed[i * 3 + 1];
                     cmap[2][i] = pColorsUsed[i * 3 + 2];
                  }
               }
               
               // Allocate Distance buffer 
               MEM_CallocMemNoFail (dist_buf,(HIST_CELLS * sizeof(unsigned long)));
               qprintf (("EntriesUsable = %d\n", EntriesUsable));            
               inv_cmap_2(EntriesUsable, cmap, HIST_BIT, dist_buf, pinvcmap);
               
               MEM_FreeMem (dist_buf);
               MEM_FreeMem (cmap[2]);
               MEM_FreeMem (cmap[1]);
               MEM_FreeMem (cmap[0]);
            
            }
            
            /*
            ** Fix up inverse color map to index actual palette.
            */
            {
               INT32 i;
               UINT8 *pinv;
               
               for (i = 0, pinv = pinvcmap; i < HIST_CELLS; i++, pinv++)
               {
                  ENSURE (*pinv < EntriesUsable);
                  *pinv = pMapFromUsed[*pinv];
               }
            }
            MEM_FreeMem (pMapFromUsed);
            MEM_FreeMem (pColorsUsed);
         }
      }
      
      if (ARG(OutPalKind))
      {
         char *psz;
         psz = (ARG(ImageOut)) ? ARG(PaletteOutFile) : ARG(OutFile);
         WriteAnyPaletteFile (psz, pColorMap, EntriesTotal, kindpal, pinvcmap, arpalsite,
            tkTransparency, TransparentIndex);
      }
      //else  
      if (ARG(ImageOut))
      {// Output palette mapped image.
         /*
         ** Use the inverse color map to palettize the first input file and write it out.
         */
         {
            LST_LIST	*listInFiles;
            LST_NODE	*pnode;
   
            listInFiles = MULTI_ARGLINKEDLIST (ARG(InFileList));
            pnode = LST_Head (listInFiles);
            ENSURE(!LST_IsEOList(pnode));
            qprintf (("Palettizing image file %s\n", LST_NodeName (pnode)));
            if (!PalettizeImageFile (LST_NodeName(pnode), ARG(OutFile), pinvcmap, pColorMap, EntriesTotal,
               tkTransparency, AlphaT, RedT, GreenT, BlueT, TransparentIndex, &EntriesUsed, imgout, dmDither,
               arpalsite)
            )
            {
               RETURN EXIT_FAILURE;
            }
            qprintf(("Actually used %d entries for image%s.\n", EntriesUsed, 
               (tkTransparency != tkNone) ? "(not counting transparency use)." : ""));
         } 
      }
      
      if (pinvcmap)
      {
         MEM_FreeMem (pinvcmap);
      }
      
      qprintf (("Done.\n"));
	}

	if (GlobalErr) {
		EL_printf ("%s\n", GlobalErrMsg);
		RETURN EXIT_FAILURE;
	}

	RETURN EXIT_SUCCESS;
}
ENDFUNCMAIN(main)

/*************************************************************************
                          BuildHistogramForFile
 *************************************************************************

   SYNOPSIS
		BOOL BuildHistogramForFile (
		   char *pszFileName,
		   HIST_ENTRY_TYPE *pHistogram
         TRANSPARENCYKIND tk,
         UINT8 Alpha,
         UINT8 Red,
         UINT8 Green,
         UINT8 Blue
		)

   PURPOSE
      To open the given file and make a histogram for it.

   INPUT
		pszFileName : Graphic file to open
		pHistogram  : Histogram table to fill in.
      tk          : Kind of transparency.
      Alpha       : Alpha threshhold for transparency (for transparency by alpha kind).
      Red         : Red value for transparency.
      Green       : Green value for transparency.
      Blue        : Blue value for transparency.
   OUTPUT
		Filled in histogram table.

   EFFECTS
		None

   RETURN
      TRUE on success. FALSE on failure.

   SEE ALSO


   HISTORY
		08/11/96 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

BOOL BuildHistogramForFile (
   char *pszFileName,
   HIST_ENTRY_TYPE *pHistogram,
   TRANSPARENCYKIND tk,
   UINT8 Alpha,
   UINT8 Red,
   UINT8 Green,
   UINT8 Blue
)
BEGINPROC (BuildHistogramForFile)
{
   BOOL fSuccess;
   BlockO32BitPixels	*pbop;

   // Clear histogram.
   memset (pHistogram, 0, (HIST_CELLS * sizeof (HIST_ENTRY_TYPE)));

   pbop = Read32BitPicture (pszFileName);
   fSuccess = pbop != NULL;
   if (fSuccess)
   {
      long int i;
      pixel32  *p32;

      for (
         i= pbop->width * pbop->height,
            p32 = pbop->rgba;      
         i;
         i--, p32++
      )
      {
         int b, g, r;
         HIST_ENTRY_TYPE *ph;
         
         switch (tk) {
         case tkNone:
            break;
         case tkAlphaLow:
            if (p32->alpha <= Alpha) continue;
            break;
         case tkAlphaHigh:
            if (p32->alpha >= Alpha) continue;
            break;
         case tkRGB:
            if (p32->red == Red && p32->green == Green && p32->blue == Blue) continue;
            break;
         }
         
         r = p32->red >> HIST_SHIFT;
         g = p32->green >> HIST_SHIFT;
         b = p32->blue >> HIST_SHIFT;

         ph = pHistogram + (r * R_STRIDE) + (g * G_STRIDE) + b;
         if (*ph < HIST_ENTRY_TYPEMAX) *ph += 1;
      }
      Free32BitPicture (pbop);
   }
   else
   {
      EL_printf("ERROR: unable to read file %s\n", pszFileName);
   }

   RETURN fSuccess;

} ENDPROC (BuildHistogramForFile)


/*************************************************************************
                             MergeHistograms
 *************************************************************************

   SYNOPSIS
		BOOL MergeHistograms ( HIST_ENTRY_TYPE *pHistogram, HIST_ENTRY_TYPE *pHistogramCrnt)

   PURPOSE
      To combine the two histograms according to the method indicated
      by the global histogram method.
      
   INPUT
		pHistogram      : Input and output
		pHistogramCrnt : Input histrogram

   OUTPUT
		pHistogram = pHistogram merged with pHistogramCrnt

   EFFECTS
		None

   RETURN
      Returns TRUE if there are colors in historgram. FALSE if historgram is empty.
      
   SEE ALSO
   

   HISTORY
		08/11/96 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

BOOL MergeHistograms ( HIST_ENTRY_TYPE *pHistogram, HIST_ENTRY_TYPE *pHistogramCrnt)
BEGINFUNC (MergeHistograms)
{
   long int i;
   long int fHasEntries;
   
   fHasEntries = 0;
   for (i = HIST_CELLS; i; i--)
   {
      long int a, b;
      a = (long)*pHistogram;
      b = (long)*pHistogramCrnt++;
      switch (HistMergeMethod){
      case histmergeSum:
         a += b;
         break;
      case histmergeMax:
         a = UTL_MAX(a,b);
         break;
      default:
         ENSURE(0);
         break;
      }

      // Clamp value.
      a = UTL_MIN(a,HIST_ENTRY_TYPEMAX);
      *pHistogram++ = (HIST_ENTRY_TYPE)a;
      fHasEntries |= a;
   }

   RETURN (fHasEntries != 0);
} ENDFUNC (MergeHistograms)

/*************************************************************************
                           PalettizeImageFile                            
 *************************************************************************

   SYNOPSIS
		BOOL PalettizeImageFile (
		   char *pszFileName,
		   char *pszOutFileName,
		   UINT8 *inv_cmap,
		   UINT8 *pColorMap,
		   int TotalColors
         TRANSPARENCYKIND tk,
         UINT8 Alpha,
         UINT8 Red,
         UINT8 Green,
         UINT8 Blue,
         UINT8 IndexT,
         int   *pNumUsed,
         PALETTE_SITE *arpalsite 
		)

   PURPOSE
      To load in the image file palettize it with the given color map using
      the given inverse color map and write it out as a palettized GFF file.
  		
  
   INPUT
		pszFileName    :
		pszOutFileName :
		inv_cmap       :
		pColorMap      :
		TotalColors    :
      tk             : Kind of transparency.
      Alpha          : Alpha threshhold for transparency (for transparency by alpha kind).
      Red            : Red value for transparency.
      Green          : Green value for transparency.
      Blue           : Blue value for transparency.
      IndexT         : Index to set tranparent pixels to.
      pNumUsed       : Pointer to int to fill in with number of entries actually used, not counting
                        transparent index.
      
   OUTPUT
		Sets *pNumUsed to actual number of entries used.
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
   TODO
      Make this handle palettized file as input.
  
   HISTORY
		08/28/96 : Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

BOOL PalettizeImageFile (
   char *pszFileName,
   char *pszOutFileName,
   UINT8 *inv_cmap,
   UINT8 *pColorMap,
   int TotalColors,
   TRANSPARENCYKIND tk,
   UINT8 Alpha,
   UINT8 Red,
   UINT8 Green,
   UINT8 Blue,
   UINT8 IndexT,
   int   *pNumUsed,
   IMGOUT imgout,
   DITHERMETHOD dmDither,
   PALETTE_SITE *arpalsite 
)
BEGINFUNC (PalettizeImageFile)
{
#define NUM_COLOR_COMPONENTS_PER_PIXEL 3
#define ERRTYPE   INT32
#define ErrValOfU8(v)   (INT32)((UINT32)(v) << 16)
#define U8OfErrVal(v)   (UINT8) ( ((UINT32)(v) >> 16) | ((v&0x80000000) ? 0xFFFF0000 : 0) )
#define ERR_0_POINT_5   (0x00008000)
   BOOL fSuccess;
   GFF	*pgff;
   UINT8 *pfUsed; // Flag wither palette entry was used.
   
   
   pgff = ReadGFF (pszFileName);
   fSuccess = (pgff != NULL);
   if (fSuccess)
   {
      int Width, Height;
      int x, y;
      RGBADATA *prgba;
      UINT8 *ppndx;
      INT32 Dimensions;
      CHUNKPNDX *pchunkpndx;
      CHUNKNODE *pchunknode;
      ERRTYPE *prErr;                  // fixed point 8.8
      ERRTYPE *prErrBuff;             // Buffer for tracking error values for each color component.
      unsigned int WidthOfErrRow;
      INT32  NumTransPixels;  // Num transparent pixels image.
      
      Width = pgff->pchunkggff->Data.Width; 
      Height = pgff->pchunkggff->Data.Height;
      Dimensions =  (INT32)Width * (INT32)Height;
      
      /*
      ** Allocate pixel index chunk node and chunk and add it to GFF
      */
      pchunknode = CreateGFFChunkNodeNoFail (IDPNDX, Dimensions);
      pchunkpndx = (CHUNKPNDX *)pchunknode->pchunk;
      LST_AddTail (pgff->plistChunkNodes, pchunknode);

      if (dmErrorPropagation == dmDither) 
      {
         UINT32 NumErrs;
         UINT32 SizeOfErrBuff;
         
         NumErrs = Dimensions * NUM_COLOR_COMPONENTS_PER_PIXEL;
         SizeOfErrBuff =  NumErrs * sizeof (ERRTYPE);
         MEM_CallocMemNoFail(prErrBuff, SizeOfErrBuff);
         prErr = prErrBuff;
      }
      WidthOfErrRow = Width * NUM_COLOR_COMPONENTS_PER_PIXEL;
      
      // Allocate color use counting table
      MEM_CallocMemNoFail (pfUsed, TotalColors);
      *pNumUsed = 0;
      
      NumTransPixels = 0;
      /* Map the image to the palette */
      for ( 
         y = 1, 
            prgba = &pgff->pchunkrgba->Data,
            ppndx = &pchunkpndx->Data;
         y <= Height; 
         y++
      )
      {
         for (
            x = 1; 
            x <= Width; 
            x++, prgba++, ppndx++, prErr += NUM_COLOR_COMPONENTS_PER_PIXEL
         )
         {
            int b, g, r;
         
            switch (tk) {
            case tkNone:
               break;
            case tkAlphaLow:
               if (prgba->Alpha <= Alpha)
               {
                  *ppndx = IndexT;
                  ++NumTransPixels;
                continue;
               }
               break;
            case tkAlphaHigh:
               if (prgba->Alpha >= Alpha) 
               {
                  *ppndx = IndexT;
                  ++NumTransPixels;
                  continue;
               }
               break;
            case tkRGB:
               if (prgba->Red == Red && prgba->Green == Green && prgba->Blue == Blue) 
               {
                  *ppndx = IndexT;
                  ++NumTransPixels;
                  continue;
               }
               break;
            } // Transparency kind switch
            
            switch (dmDither) {
            case dmNone:
               r = prgba->Red >> HIST_SHIFT;
               g = prgba->Green >> HIST_SHIFT;
               b = prgba->Blue >> HIST_SHIFT;
               *ppndx = inv_cmap[(r * R_STRIDE) + (g * G_STRIDE) + b];
               break;
            case dmErrorPropagation:   
               {
                  UINT8 aru8ComponentNew[NUM_COLOR_COMPONENTS_PER_PIXEL];
                  UINT8 *pu8ComponentSrc;
                  UINT8 *pu8ComponentDst;
                  ERRTYPE *prErrCrnt;
                  ERRTYPE *prErrNextRow; // fixed point 8.8
                  int c;
                  
                  // Add error into components to come up with new color to me mapped.
                  for (
                     c = NUM_COLOR_COMPONENTS_PER_PIXEL,
                        pu8ComponentSrc = &prgba->Red,
                        pu8ComponentDst = aru8ComponentNew,
                        prErrCrnt = prErr,
                        prErrNextRow = prErr + WidthOfErrRow; 
                     c; 
                     c--,
                        pu8ComponentSrc++,
                        pu8ComponentDst++,
                        prErrCrnt++,
                        prErrNextRow++
                  )
                  {
                     ERRTYPE rValue;
                     UINT8 u8Value;
                     
                     rValue = ErrValOfU8(*pu8ComponentSrc) + *prErrCrnt;
                     if (rValue < ErrValOfU8(0))
                     {
                        u8Value = 0;
                     }
                     else if (rValue >= ErrValOfU8(0xFF))
                     {
                        u8Value = 0xFF;
                     }
                     else
                     {
                        /* Round up to whole value */
                        rValue = (rValue + ERR_0_POINT_5);  // add 0.5 to round up to high 5 bits.
                        u8Value = U8OfErrVal(rValue);
                     }
                     *pu8ComponentDst = u8Value & 0xFF; 
                  } // End Component loop
                  // Map the resultant error propagated color to the palette  
                  r = aru8ComponentNew[0] >> HIST_SHIFT;
                  g = aru8ComponentNew[1] >> HIST_SHIFT;
                  b = aru8ComponentNew[2] >> HIST_SHIFT;
                  *ppndx = inv_cmap[(r * R_STRIDE) + (g * G_STRIDE) + b];
            
                  // Fill temporary  array with actual color values from palette to which this pixel was mapped.
                  {
                     UINT8 *pu8PaletteColor;
                     pu8PaletteColor = pColorMap + (*ppndx * NUM_COLOR_COMPONENTS_PER_PIXEL);
                     aru8ComponentNew[0] = pu8PaletteColor[0];
                     aru8ComponentNew[1] = pu8PaletteColor[1];
                     aru8ComponentNew[2] = pu8PaletteColor[2];
                  }
                  
                  // Calculate and spread out the error for each color component.
                  for (
                     c = NUM_COLOR_COMPONENTS_PER_PIXEL,
                        pu8ComponentSrc = &prgba->Red,
                        pu8ComponentDst = aru8ComponentNew,
                        prErrCrnt = prErr,
                        prErrNextRow = prErr + WidthOfErrRow; 
                     c; 
                     c--,
                        pu8ComponentSrc++,
                        pu8ComponentDst++,
                        prErrCrnt++,
                        prErrNextRow++
                  )
                  {
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
                        //      Right Edge?         Left Edge?      Bottom Edge?
                        icase = ((Width==x) << 2) | ((1==x) << 1) | (Height==y);
                        switch (icase) { 
                        case 0: // Current pixel not on any edge of image
                           prErrCrnt[NUM_COLOR_COMPONENTS_PER_PIXEL]     += rError * 7/16;   //       +-+-+-+
                           prErrNextRow[-NUM_COLOR_COMPONENTS_PER_PIXEL] += rError * 3/16;   //       |x|*|1|
                           prErrNextRow[0]                               += rError * 5/16;   //       +-+-+-+
                           prErrNextRow[NUM_COLOR_COMPONENTS_PER_PIXEL]  += rError * 1/16;   //       |2|3|4|
                           break;                                                            //       +-+-+-+
                        case 1: // Current pixel on bottom edge of image.
                           prErrCrnt[NUM_COLOR_COMPONENTS_PER_PIXEL]     += rError * 7/16;   //       +-+-+-+
                           //prErrNextRow[-NUM_COLOR_COMPONENTS_PER_PIXEL] += rError * 3/16; //       |x|*|1|
                           //prErrNextRow[0]                               += rError * 5/16; //       +-+-+-+
                           //prErrNextRow[NUM_COLOR_COMPONENTS_PER_PIXEL]  += rError * 1/16; //       |#|#|#|
                           break;                                                            //       +-+-+-+
                        case 2: // Current pixel on left edge of image.
                           prErrCrnt[NUM_COLOR_COMPONENTS_PER_PIXEL]     += rError * 7/16;   //       +-+-+-+
                           //prErrNextRow[-NUM_COLOR_COMPONENTS_PER_PIXEL] += rError * 3/16; //       |#|*|1| 
                           prErrNextRow[0]                               += rError * 5/16;   //       +-+-+-+ 
                           prErrNextRow[NUM_COLOR_COMPONENTS_PER_PIXEL]  += rError * 1/16;   //       |#|3|4| 
                           break;                                                            //       +-+-+-+ 
                        case 3: // Current pixel on bottom left corner of image.
                           prErrCrnt[NUM_COLOR_COMPONENTS_PER_PIXEL]     += rError * 7/16;   //       +-+-+-+
                           //prErrNextRow[-NUM_COLOR_COMPONENTS_PER_PIXEL] += rError * 3/16; //       |#|*|1| 
                           //prErrNextRow[0]                               += rError * 5/16; //       +-+-+-+ 
                           //prErrNextRow[NUM_COLOR_COMPONENTS_PER_PIXEL]  += rError * 1/16; //       |#|#|#| 
                           break;                                                            //       +-+-+-+ 
                        case 4: // Current pixel on right edge of image.
                           //prErrCrnt[NUM_COLOR_COMPONENTS_PER_PIXEL]     += rError * 7/16; //       +-+-+-+
                           prErrNextRow[-NUM_COLOR_COMPONENTS_PER_PIXEL] += rError * 3/16;   //       |x|*|#| 
                           prErrNextRow[0]                               += rError * 5/16;   //       +-+-+-+ 
                           //prErrNextRow[NUM_COLOR_COMPONENTS_PER_PIXEL]  += rError * 1/16; //       |2|3|#| 
                           break;                                                            //       +-+-+-+ 
                        case 5: // Current pixel on bottom right corner of image.   
                           //prErrCrnt[NUM_COLOR_COMPONENTS_PER_PIXEL]     += rError * 7/16; //       +-+-+-+
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
               }
               break;
            } // Dither Mode switch
          
            ENSURE (*ppndx < TotalColors);
            // Count the colors actually used.
            if (!pfUsed[*ppndx])
            {
               *pNumUsed += 1;
               pfUsed[*ppndx] = TRUE;
            }
            
          
         } // End x loop
      } // End y loop
      
      MEM_FreeMem (pfUsed);
      
      if (dmErrorPropagation == dmDither) 
      {
         MEM_FreeMem(prErrBuff);
      }
      
      /* Delete the rgba chunk */
      {
         CHUNKNODE *pchunknodeRGBA;
         pchunknodeRGBA = PChunkNodeOfId (pgff, IDRGBA);
         ENSURE_PTR (pchunknodeRGBA);
         LST_Remove (pchunknodeRGBA);
         DestroyGFFChunkNode (pchunknodeRGBA);
      }
      /*
      ** Allocate palette chunk node and chunk and add it to GFF
      */
      {
         int i;
         PCONDATA *ppcon;
         PALETTE_SITE   *ppalsite;
         CHUNKPCON *pchunkpcon;
         
         pchunknode = CreateGFFChunkNodeNoFail (IDPCON, TotalColors * sizeof(PCONDATA));
         pchunkpcon = (CHUNKPCON *)pchunknode->pchunk;
         LST_AddTail (pgff->plistChunkNodes, pchunknode);
         /* Fill in pcon with pallete data */
         for (i = 0, ppcon = &pchunkpcon->Data, ppalsite = arpalsite;
            i < TotalColors;
            i++, ppcon++, ppalsite++
         )
         {
            ppcon->Red = *pColorMap++;
            ppcon->Green = *pColorMap++;
            ppcon->Blue = *pColorMap++;
            ppcon->Constraint = 
                 ((skOpen == ppalsite->SiteKind) ? 0 : GFF_PCON_NOT_SETTABLE)
               | ((skBlocked == ppalsite->SiteKind) ? GFF_PCON_NOT_USABLE : 0)
               | ((tkNone != tk && i == IndexT) ? GFF_PCON_TRANSPARENT : 0)
            ;
         }
      }

      switch (imgout) {
      case ioNone:
         break;
      case ioLRTB:
         {
            int fh;
            fh = CHK_WriteOpen (pszOutFileName);
            CHK_Write (fh, &pchunkpndx->Data, Dimensions);
            CHK_Close (fh);
         }
         break;
      case ioGFF:
         WriteGFF (pszOutFileName, pgff);
         break;
      case ioCLRTB:
      case ioCDLRTB:
         {            
            int fh;
            UINT32 fs;
            fs = 0;
            fs |= (NumTransPixels == Dimensions) ? CODEDRAW_ALL_TRANSPARENT : 0;
            fs |= (NumTransPixels) ? CODEDRAW_SOME_TRANSPARENT : 0;
            
            qprintf (("Transparent pixels = %ld\n", (long int)NumTransPixels));
            fh = CHK_WriteOpen (pszOutFileName);
            if (fBigEndian)
            {
               fs = NativeToMSBF32Bit(fs);
            }
            else
            {
               fs = NativeToLSBF32Bit(fs);
            }
            CHK_Write (fh, &fs, 4);
            
            /* Write width and height if needed */
            if (ioCDLRTB == imgout)
            {
               UINT16 w, h;
               w = (UINT16)Width;
               h = (UINT16)Height;
               w = (fBigEndian) ? (NativeToMSBF16Bit(w)) : (NativeToLSBF16Bit(w));
               h = (fBigEndian) ? (NativeToMSBF16Bit(h)) : (NativeToLSBF16Bit(h));
               
               CHK_Write (fh, &w, 2);
               CHK_Write (fh, &h, 2);
            }
            
            if (!(fs & CODEDRAW_ALL_TRANSPARENT))
            {
               CHK_Write (fh, &pchunkpndx->Data, Dimensions);
            }
            CHK_Close (fh);
         }
         break;         
      case ioSony4BitCLRTB:
         {            
            int fh;
            UINT32 fs;
            fs = 0;
            fs |= (NumTransPixels == Dimensions) ? CODEDRAW_ALL_TRANSPARENT : 0;
            fs |= (NumTransPixels) ? CODEDRAW_SOME_TRANSPARENT : 0;
            
            qprintf (("Transparent pixels = %ld\n", (long int)NumTransPixels));
            fh = CHK_WriteOpen (pszOutFileName);
            if (fBigEndian)
            {
               fs = NativeToMSBF32Bit(fs);
            }
            else
            {
               fs = NativeToLSBF32Bit(fs);
            }
            CHK_Write (fh, &fs, 4);
            
            if (!(fs & CODEDRAW_ALL_TRANSPARENT))
            {
               int i;
               UINT8 *pdata;
               pdata = &pchunkpndx->Data;
               for (i = Dimensions; i; i-=2)
               {
                  UINT8 data;
                  
                  ENSURE (pdata[0] < 16);
                  ENSURE (pdata[1] < 16);
                  data = pdata[0] | (pdata[1] << 4);
                  CHK_Write (fh, &data, 1);
                  pdata += 2;
               }
               //if (!(fs & CODEDRAW_ALL_TRANSPARENT))
               //{
                //  CHK_Write (fh, &pchunkpndx->Data, Dimensions);
               //}
            }
            CHK_Close (fh);
         }
         break;         
      default:
         ENSURE(0);
         break;                  
      }
            
      
      FreeGFF(pgff);
   }
   else
   {
      EL_printf("ERROR: unable to read file %s\n", pszFileName);
   }

   RETURN fSuccess;
} ENDFUNC (PalettizeImageFile)


/*************************************************************************
                           WriteAnyPaletteFile                           
 *************************************************************************

   SYNOPSIS
		BOOL WriteAnyPaletteFile (
		   char *pszOutFile, 
		   uint8 *pColorMap, 
		   int TotalColors, 
		   KindPal kindpal,
         UINT8 *pinvcmap,
         PALETTE_SITE *arpalsite,
         TRANSPARENCYKIND tk,
         UINT8 IndexT
		)

   PURPOSE
  		To write whatever kind of palette file is indicated by rawpal
  		
  
   INPUT
		pszOutFile  : File name to use for file output
		pColorMap   :
		TotalColors :
		kindpal     :
      pinvcmap    : Inverse color map.
      arpalsite   : palette site array to set kinds for.
      tk          : Kind of transparency.
      IndexT      : Index to set tranparent pixels to.
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		11/20/96 : Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

BOOL WriteAnyPaletteFile (
   char *pszOutFile, 
   uint8 *pColorMap, 
   int TotalColors, 
   KINDPAL kindpal,
   UINT8 *pinvcmap,
   PALETTE_SITE *arpalsite,
   TRANSPARENCYKIND tk,
   UINT8 IndexT
)
BEGINFUNC (WriteAnyPaletteFile)
{
	BOOL fSuccess;
   
   fSuccess = TRUE;
   switch (kindpal) {
   case kpNone:
      break;
   case kpGFF:
      fSuccess = WritePaletteFile (pszOutFile, pColorMap, TotalColors, pinvcmap, arpalsite, tk, IndexT);
      break;
   case kpRGB:      
      {
         int fh;
         
         fh = CHK_WriteOpen (pszOutFile);
         CHK_Write (fh, pColorMap, (TotalColors * 3));
         CHK_Close (fh);
      }
      break;   
   case kpDirectX:      
      {
         int fh;
         int i;
         UINT8 *pu8RGBFBuf, *pu8RGBF;
         unsigned int SizeOfDirectXPalette;
         
         SizeOfDirectXPalette = TotalColors * 4;
         MEM_CallocMemNoFail (pu8RGBFBuf, SizeOfDirectXPalette);
         for (i = TotalColors, pu8RGBF = pu8RGBFBuf; i; i--)
         {
            *pu8RGBF++ = *pColorMap++;
            *pu8RGBF++ = *pColorMap++;
            *pu8RGBF++ = *pColorMap++;
            pu8RGBF++; // skip flag byte which is already zero.
         }
         
         fh = CHK_WriteOpen (pszOutFile);
         CHK_Write (fh, pu8RGBFBuf, SizeOfDirectXPalette);
         CHK_Close (fh);
         MEM_FreeMem (pu8RGBFBuf);
      }
      break;
   case kpSony16:
      {
         int fh;
         int i;
         
         fh = CHK_WriteOpen (pszOutFile);
         for (i = 0;  i < 16; i+= 2)
         {
            UINT16 psx1, psx2;
            UINT32 psxlong; 
            UINT16 red, green, blue;
            
            psx1 = 0;
            red = pColorMap[0];
            green = pColorMap[1];
            blue = pColorMap[2];
            red >>= 3;
            green >>= 3;
            blue >>= 3;
            psx1 = red | (green << 5) | (blue << 10);
            if (i != IndexT)
            {
               psx1 |= 0x8000;  // set translucent flag bit
            }
            pColorMap += 3;
            
            
            psx2 = 0;
            red = pColorMap[0];
            green = pColorMap[1];
            blue = pColorMap[2];
            red >>= 3;
            green >>= 3;
            blue >>= 3;
            psx2 = red | (green << 5) | (blue << 10);
            if ((i+1) != IndexT)
            {
               psx2 |= 0x8000;  // set translucent flag bit
            }
            pColorMap += 3;
            
            
            psxlong =(psx2 << 16) | (psx1);
            CHK_Write (fh, &psxlong, 4);
         }
         CHK_Close (fh);
      
      }
      
      break;      
   case kpKludge:
      {
         int i;
         
         EL_printf("\n\n {\n");
         for (i = TotalColors;  i; i-= 2)
         {
            UINT16 psx1, psx2;
            UINT32 psxlong; 
            UINT16 red, green, blue;
            
            psx1 = 0;
            red = pColorMap[0];
            green = pColorMap[1];
            blue = pColorMap[2];
            red >>= 3;
            green >>= 3;
            blue >>= 3;
            psx1 = red | (green << 5) | (blue << 10);
            
            
            pColorMap += 3;
            
            
            psx2 = 0;
            red = pColorMap[0];
            green = pColorMap[1];
            blue = pColorMap[2];
            red >>= 3;
            green >>= 3;
            blue >>= 3;
            psx2 = red | (green << 5) | (blue << 10);
            
            
            pColorMap += 3;
            
            psxlong =(psx2 << 16) | (psx1);
            if (i == TotalColors)
            {
               psxlong |= 0x80000000;
            }
            else
            {
               psxlong |= 0x80008000;
            }
            EL_printf ("0x%08lX,\n", (unsigned long)psxlong);
            
         }
         EL_printf ("};\n\n");
      
      }
      
      break;      
   default:
      ENSURE(0);
      break;                  
   }

	RETURN fSuccess;
} ENDFUNC (WriteAnyPaletteFile)
  

/*************************************************************************
                            WritePaletteFile                             
 *************************************************************************

   SYNOPSIS
		BOOL WritePaletteFile (
         char *pszOutFile, 
         uint8 *pColorMap, 
         int TotalColors,
         UINT8 *pinvcmap,
         PALETTE_SITE *arpalsite, 
         TRANSPARENCYKIND tk,
         UINT8 IndexT
      )

   PURPOSE
  		To write out a GFF file with just a palette.
  
   INPUT
		pszOutFile  :
		pColorMap   :
		TotalColors :
      pinvcmap    : Inverse color map.
      arpalsite   : palette site array to set kinds for.
      tk          : Kind of transparency.
      IndexT      : Index to set tranparent pixels to.
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		08/28/96 : Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

BOOL WritePaletteFile (
   char *pszOutFile, 
   uint8 *pColorMap, 
   int TotalColors,
   UINT8 *pinvcmap,
   PALETTE_SITE *arpalsite,
   TRANSPARENCYKIND tk,
   UINT8 IndexT
)
BEGINFUNC (WritePaletteFile)
{
	BOOL fSuccess = TRUE;
   GFF *pgff;
   CHUNKNODE *pchunknode;
   CHUNKGGFF *pchunkggff;
   CHUNKPCON *pchunkpcon;
   CHUNKINVP *pchunkinvp;
   
   pgff = CreateGFFNoFail();            
   
   /* Make GGFF chunk */
   pchunknode = CreateGFFChunkNodeNoFail (IDGGFF, sizeof(GGFFDATA));
   LST_AddTail (pgff->plistChunkNodes, pchunknode);
   pchunkggff = (CHUNKGGFF *)pchunknode->pchunk;
   pgff->pchunkggff = pchunkggff;
   pchunkggff->Data.ByteOrder  = GFF_BYTE_ORDER;
   pchunkggff->Data.Width = 0;
   pchunkggff->Data.Height = 0;
   
   /* Make PCON chunk */
   pchunknode = CreateGFFChunkNodeNoFail (IDPCON, TotalColors * sizeof (PCONDATA));
   LST_AddTail (pgff->plistChunkNodes, pchunknode);
   pchunkpcon = (CHUNKPCON *) pchunknode->pchunk;
   pgff->pchunkpcon = pchunkpcon;
   {
      int i;
      PCONDATA *pcondata;
      PALETTE_SITE   *ppalsite;
      
      for (i = 0, pcondata = &pchunkpcon->Data, ppalsite = arpalsite; 
         i < TotalColors; 
         i++, pcondata++, ppalsite++
      )
      {
         pcondata->Red = *pColorMap++;
         pcondata->Green = *pColorMap++;
         pcondata->Blue = *pColorMap++;
         pcondata->Constraint = 
              ((skOpen == ppalsite->SiteKind) ? 0 : GFF_PCON_NOT_SETTABLE)
            | ((skBlocked == ppalsite->SiteKind) ? GFF_PCON_NOT_USABLE : 0)
            | ((tkNone != tk && i == IndexT) ? GFF_PCON_TRANSPARENT : 0)
         ;
      }
   }
   
   /* Make INVP Chunk */
   pchunknode = CreateGFFChunkNodeNoFail (IDINVP, HIST_CELLS);
   LST_AddTail (pgff->plistChunkNodes, pchunknode);
   pchunkinvp = (CHUNKINVP *) pchunknode->pchunk;
   memcpy (&pchunkinvp->Data, pinvcmap, HIST_CELLS);


   
   WriteGFF(pszOutFile, pgff);
   FreeGFF (pgff);
   
	RETURN fSuccess;
} ENDFUNC (WritePaletteFile)

/*************************************************************************
                               ParseRangeList                               
 *************************************************************************

   SYNOPSIS
		BOOL ParseRangeList (
         LST_LIST *plist, 
         PALETTE_SITE *arpalsite, 
         SITEKIND SiteKind 
         int *pNumSet,
         int *pEntryHighest
         )

   PURPOSE
  		To parse a sequence of palette index ranges and assign the given
      SiteKind to the corresponding entries in arpalsite.
  
   INPUT
		plist         : List of strings specifying ranges.
      arpalsite     : palette site array to set kinds for.
		SiteKind      : What kind to set these entries to.
      pNumSet       : Poniter to var to fill in with num entries affected.
      pEntryHighest : Highest index set.
      
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
      TRUE on success. FALSE on failure.
  
   SEE ALSO
  
  
   HISTORY
		08/27/96 : Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

BOOL ParseRangeList (
   LST_LIST *plist, 
   PALETTE_SITE *arpalsite, 
   SITEKIND SiteKind, 
   int *pNumSet,
   int *pEntryHighest
)
BEGINFUNC (ParseRangeList)
{
   LST_NODE	*pnode;

   *pNumSet = 0;
   *pEntryHighest = 0;
   qprintf (("Indexes:\n"));
   for (
      pnode = LST_Head (plist);
      !LST_IsEOList(pnode);
      pnode = LST_Next (pnode)
   )
   {
      if (!ParseRange( LST_NodeName (pnode), arpalsite, SiteKind, pNumSet, pEntryHighest))
      {
         RETURN FALSE;
      }
   }
   
   RETURN TRUE;
} ENDFUNC (ParseRangeList)

/*************************************************************************
                               ParseRange                                
 *************************************************************************

   SYNOPSIS
		BOOL ParseRange (
		   char *psz, 
		   PALETTE_SITE *arpalsite, 
		   SITEKIND SiteKind, 
		   int *pNumSet,
		   int *pEntryHighest
		)

   PURPOSE
  		To parse a palette index range string and assign the given
      SiteKind to the corresponding entries in arpalsite.
  		
  
   INPUT
		psz           : String in format indicated in arparse help above.
      arpalsite     : palette site array to set kinds for.
		SiteKind      : What kind to set these entries to.
      pNumSet       : Poniter to var to fill in with num entries affected.
      pEntryHighest : Highest index set.
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
      TRUE on success. FALSE on failure.
  
  
   SEE ALSO
  
  
   HISTORY
		02/28/97 : Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

BOOL ParseRange (
   char *psz, 
   PALETTE_SITE *arpalsite, 
   SITEKIND SiteKind, 
   int *pNumSet,
   int *pEntryHighest
)
BEGINFUNC (ParseRange)
{
   int i, j, NumArgs;
   
   qprintf ((" %s\n", psz));
   NumArgs = sscanf (psz, "%d..%d", &i, &j);
   if (NumArgs < 1) {
      EL_printf ("Error: Bad palette restriction format '%s'\n", psz);
      RETURN FALSE;
   }
   if (1 == NumArgs) 
   {
    j = i;
   } 
   else 
   {
      if (j == i)
      {
         EL_printf ("WARNING: Range '%s' covers only one value.\n", psz);
      }
      else if (j < i) 
      {
         int temp;
         EL_printf ("WARNING: Range '%s' in unusual order.\n", psz);
         temp = i;
         i = j;
         j = temp;
      }
   }
   if (i < 0 || i >= ENTRIES_MAX || j < 0 || j >= ENTRIES_MAX)
   {
      EL_printf ("ERROR: Value '%s' is out of bounds (0..%d).\n", psz, ENTRIES_MAX-1);
      RETURN FALSE;
   }
   qprintf (("%d-%d\n", i, j));
   if (j > *pEntryHighest)
   {
      *pEntryHighest = j;
   }
   for (  ;i <= j ; i++, *pNumSet += 1)
   {
      if (arpalsite[i].SiteKind != skOpen)
      {
         EL_printf ("ERROR: Palette index %d is assigned conflicting states of 'blocked' and 'constant'.\n", i);
         RETURN FALSE;
      }
      arpalsite[i].SiteKind = SiteKind;
      #if 0
      qprintf(("arpalsite[%d]: (%d), %d,%d,%d\n", i, (int)SiteKind, 
         (int)arpalsite[i].r, (int)arpalsite[i].g, (int)arpalsite[i].b));
      #endif            
   }
      
   RETURN TRUE;
   
} ENDFUNC (ParseRange)

/*************************************************************************
                              ReadAPalette                               
 *************************************************************************

   SYNOPSIS
		UINT8 *ReadAPalette (
		   char *pszPaletteFile,
		   int *pNumInPalette,
		   UINT8 **ppinvcmap
		)

   PURPOSE
  		Read a palette from a file and if it is a GFF file get the inverse color
      map if it is present.
  
   INPUT
		pszPaletteFile :
		pNumInPalette  :
		ppinvcmap      :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		02/28/97 : Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

UINT8 *ReadAPalette (
   char *pszPaletteFile,
   int *pNumInPalette,
   UINT8 **ppinvcmap
)
BEGINFUNC (ReadAPalette)
{
   UINT8 *pu8;
   
   pu8 = NULL;
   
   if (!stricmp (".gff", EIO_Ext(pszPaletteFile)))
   {
   	MEMFILE				*mf;
   
   	mf = MEMFILE_Load (pszPaletteFile);
   	if (mf)
   	{
   
         CHUNKHEADER   chunkheader;
         int result, headerresult;;
         BOOL  fGotPCON, fGotINVP;
         
         fGotPCON = FALSE;
         fGotINVP = FALSE;
         
         /* Read chunks until find pcon chunk and invp chunk */
         while (!fGotPCON || !fGotINVP)
         {
            headerresult = MEMFILE_Read (mf, &chunkheader, sizeof (CHUNKHEADER));
            
            if (sizeof (CHUNKHEADER) != headerresult)
            {
               break;
            } 
            else if (!fGotPCON && chunkheader.id == IDPCON)
            { /* Found pcon chunk */
               int i;
               UINT8 *pu8Temp;
               
		         MEM_CallocMemNoFail(pu8, 3*256);
               
               for (i = 0, pu8Temp = pu8; i < 256; i++, pu8Temp += 3)
               {
                  int result;
                  PCONDATA pcondata;
                  result = MEMFILE_Read (mf, &pcondata, sizeof(PCONDATA));
                  if (result != sizeof(PCONDATA)) break;
                  pu8Temp[0] = pcondata.Red; 
                  pu8Temp[1] = pcondata.Green; 
                  pu8Temp[2] = pcondata.Blue; 
               }
               *pNumInPalette = i;
               fGotPCON = TRUE;
            }
            else if (!fGotINVP && chunkheader.id == IDINVP)
            { /* Found invp chunk */
               UINT8 *pinvcmap;
               
               qprintf(("Found inverse palette in GFF palette file.\n"));
               if (chunkheader.Size == HIST_CELLS)
               {
                  MEM_AllocMemNoFail (pinvcmap, HIST_CELLS);
                  result = MEMFILE_Read (mf, pinvcmap, HIST_CELLS);
                  if (HIST_CELLS == result)
                  {
                     *ppinvcmap = pinvcmap;
                     fGotINVP = TRUE;
                  }
                  else
                  {
                     qprintf(("WARNING: Error reading inverse color map from GFF file.\n Inverse color map will be generated.\n"));
                  }
               }
               else
               {
                  qprintf(("WARNING: Incompatibly sized inverse color map in GFF file.\n Inverse color map will be generated.\n"));
               }
            }
            else // skip unwanted data.
            {
               MEMFILE_Seek (mf, chunkheader.Size, SEEK_CUR);
            }
         }
		   MEMFILE_Close (mf);
      }  
   }
   else
   {
      pu8 = ReadRawPalette (pszPaletteFile, pNumInPalette);
   }

	RETURN pu8;
   
} ENDFUNC (ReadAPalette)
