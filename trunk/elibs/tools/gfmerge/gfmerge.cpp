
/*************************************************************************
 *                                                                       *
 *                            GFMERGE.CPP                                *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION
   
      Takes GFF file(s) with palettes and merges them into one output 
      palette.  Uses constraint info in PCON chunk to figure out how to 
      merge them.
      

   PROGRAMMERS
      Juan M. Alvarado

   FUNCTIONS

   TABS : 4 7

   HISTORY
		03/19/97 : JMA Created. (Hacked out of gfpal.cpp)
                  
            
   TODO
            
               
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
#include <echidna\listapi.h>

#include "inv_cmap.h"

/*************************** C O N S T A N T S ***************************/

#define ENTRIES_MAX  256
#define ENTRY_INDEX_MAX  (ENTRIES_MAX - 1)

/*----------------------------------------------------------------------------*/
#define HIST_BIT   (6)
#define HIST_MAX   (1 << HIST_BIT)
#define R_STRIDE   (HIST_MAX * HIST_MAX)
#define G_STRIDE   (HIST_MAX)
#define HIST_SHIFT (8 - HIST_BIT)
#define HIST_CELLS (HIST_MAX * HIST_MAX * HIST_MAX)
#define HIST_ENTRY_TYPE          UINT16
#define HIST_ENTRY_TYPEMAX       UINT16MAX

enum {
   codeOpen,        // palette site is     Usable and     Settable.
   codeConst,       // palette site is     Usable but NOT Settable.
   codeBarren,      // palette site is NOT Usable but     Settable.
   codeBlocked,     // palette site is NOT Usable and NOT Settable.
};

#if (GFF_PCON_NOT_SETTABLE != (1 << 0))
   #error code enums need updating         
#endif
#if (GFF_PCON_NOT_USABLE != (1 << 1))
   #error code enums need updating         
#endif  
  

/******************************* T Y P E S *******************************/
typedef enum {
   tkNone,     // No transparency in this image
   tkAlphaLow,  // Transparent if Alpha below given threshhold
   tkAlphaHigh,  // Transparent if Alpha above given threshhold
   tkRGB       // Transparent if Red, Green and Blue component equal given values.
} TRANSPARENCYKIND;

typedef enum {
   kpMinex = -2,
   kpNone,        // no palette file output.
   kpRGB,         // raw output RGB format.
   kpGFF,         // GFF format.
   kpDirectX,     // raw output left right top to bottom
   kpKludge,
   kpMaxex
} KINDPAL;

typedef enum {
   methMinex = -1,
   methSplice,     
   methMaxex
} METHOD;

typedef struct {
   LST_NODE  node;          // filename in LST_NodeName(&node)
   int NumInPalette;    // Num entries in palette.
   PCONDATA *ppcondata;
} PALREC;   

typedef enum {
   mkOpen,        // not yet set or referenced by any palette
   mkConstRef,    // referenced as consant by some palette. 
   mkSet         // Set by some palette
} MERGEKIND;

typedef struct {
   PCONDATA pcon;
   MERGEKIND   mergekind;
   char *psz;        // Pointer to palette file name that set this position
} MERGE;

/************************** P R O T O T Y P E S **************************/

PALREC *ReadAPalette (
   PALREC *ppalrec
);

BOOL WritePaletteFile (
   char *pszOutFile, 
   int TotalColors, 
   UINT8 *pinvcmap,
   PCONDATA *ppcondata
   );
   
BOOL WriteAnyPaletteFile (
   char *pszOutFile, 
   int TotalColors, 
   KINDPAL kindpal, 
   UINT8 *pinvcmap,
   PCONDATA *ppcondata
   );

BOOL SplicePalettes (LST_LIST *plistpalrec, PCONDATA *ppcondataNew);

/***************************** G L O B A L S *****************************/

/****************************** M A C R O S ******************************/

/**************************** R O U T I N E S ****************************/


/*************************** ArgParse Template ***************************/
enum {
   NDX_InverseCMap,
   NDX_OutPalKind,
   NDX_Method,
   NDX_Quiet,
   NDX_OutFile,
   NDX_InFileList,
};
#define ARG(name) (newargs [NDX_ ## name])
#define qprintf(arg_list)  (!ARG(Quiet)) ? EL_printf arg_list : NULL

static char Usage[] = "Usage: GfMerge [Switches] OUTFILE INFILES\n";
static char	**newargs;
   
// Guide or keeping help to 80 columns:   
//    "         1         2         3         4         5         6         7         8"
//    "12345678901234567890123456789012345678901234567890123456789012345678901234567890"
ArgSpec Template[] = {
   {CHRSWITCH_ARG, "I",        
      "    -I             Inverse Color Map.  Create and store it with GFF palette.\n"
   ,},
   {CHRKEYWORD_ARG, "K",
      "    -K<code>       Write output palette file of type <mode>: (See -X):\n"
      "                      code Description\n"
      "                      ---- -----------\n"
      "                       0   Raw RGB: 3 bytes per entry, 1 byte per compnent\n"
      "                       1   GFF file (Default).\n"
      "                       2   Direct X palette.\n"
   ,},
   {CHRKEYWORD_ARG, "M",	      
      "    -M<method>     Method of combining palettes:\n"
      "                      code Description\n"
      "                      ---- -----------\n"
      "                       0   Splice using palette contraint info (Default).\n"
   ,},
   {CHRSWITCH_ARG, "Q",        
      "    -Q             Quiet. No progress printing.\n"
   ,},
   {STANDARD_ARG|REQUIRED_ARG, "OUTFILE",	
      "    OUTFILE        Output palette file:\n" 
   ,},
   {STANDARD_ARG|REQUIRED_ARG|MULTI_ARG|LIST_ARG,  "INFILES",	
      "    INFILES        GFF palette file(s) to read.\n"
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
      #if 0
      UINT8 *pColorMap;                   // Actual output palette for image. May have colors not used by image.
      PALETTE_SITE *arpalsite;            // Array that describes the usage status of each palette entry.
      
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
      DITHERMETHOD   dmDither;      
      #endif
      
      KINDPAL  kindpal;                   // What kind of output palette file to write. 
      PCONDATA *ppcondataNew;             // New combined palette.
      int EntriesTotal;                   // Number of entries in final palette.
      UINT8 *pinvcmap;                    // Pointer to inverse color map.
      LST_LIST   listpalrec;
      LST_LIST   *plistpalrec;
      METHOD   meth;
      
      pinvcmap = NULL;
      EntriesTotal = ENTRIES_MAX;      // for now.
      plistpalrec = &listpalrec;
      
      LST_InitList (plistpalrec);
      
      
      kindpal = kpGFF;
      if (ARG (OutPalKind)) 
      {
         kindpal = (KINDPAL)atoi(ARG (OutPalKind));
         ENSURE_((kpNone < kindpal && kindpal < kpMaxex), "Palette output kind is out of range");
      }
      
      meth = methSplice;
      if (ARG(Method))
      {
         meth = (METHOD)atoi(ARG (Method));
         ENSURE_((methMinex < meth && meth < methMaxex), "Method code is out of range");
      }

      /*
      ** Allocate new palette.
      */
      MEM_CallocMemNoFail (ppcondataNew, (ENTRIES_MAX * sizeof(PCONDATA)));
      
      /*
      ** Read input palettes.
      */
      {
         LST_LIST	*plistInFiles;
         LST_NODE	*pnode;
         PALREC   *ppalrec;
      
         plistInFiles = MULTI_ARGLINKEDLIST (ARG(InFileList));
         qprintf (("Reading Palettes:\n"));
         for (pnode = LST_Head (plistInFiles); !LST_IsEOList(pnode); pnode = LST_Next (pnode))
         {
            qprintf (("   %s\n", LST_NodeName (pnode)));
            ppalrec = (PALREC *)LST_CreateNode(sizeof(PALREC), LST_NodeName(pnode));
            LST_AddTail (plistpalrec, ppalrec);
            
            if (!ReadAPalette (ppalrec))
            {
               RETURN EXIT_FAILURE;
            }
         }
      }
      
      
      /*
      ** Combine the palettes according to the method 
      */
      switch (meth) {
      case methSplice:
         qprintf(("Combining by splicing\n"));
         if (!SplicePalettes (plistpalrec, ppcondataNew))
         {
            ErrMess("Unable to splice together palettes.\n");
         }
         break;
      default:
         ENSURE_(0, "Unknown method");
      }
      
      /* Make inverse color map if requested */
      if (ARG(InverseCMap))
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
         
            MEM_CallocMemNoFail (pColorsUsed, (ENTRIES_MAX * 3));
         
            /*
            ** Build temp palette for feeding to inverse color map routine.  
            */
            {
               int i;
               UINT8 *pUsed;
               PCONDATA *ppcon;
               
               pUsed = pColorsUsed;
               for (i = 0, ppcon = ppcondataNew; 
                  i < EntriesTotal; 
                  i++, ppcon++)
               {
                  {
                     *pUsed++ = ppcon->Red;
                     *pUsed++ = ppcon->Green;
                     *pUsed++ = ppcon->Blue;
                  }
               }
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
                  for (i = 0; i < EntriesTotal; i++)
                  {
                     cmap[0][i] = pColorsUsed[i * 3 + 0];
                     cmap[1][i] = pColorsUsed[i * 3 + 1];
                     cmap[2][i] = pColorsUsed[i * 3 + 2];
                  }
               }
               
               // Allocate Distance buffer 
               MEM_CallocMemNoFail (dist_buf,(HIST_CELLS * sizeof(unsigned long)));
               qprintf (("EntriesTotal = %d\n", EntriesTotal));            
               inv_cmap_2(EntriesTotal, cmap, HIST_BIT, dist_buf, pinvcmap);
               
               MEM_FreeMem (dist_buf);
               MEM_FreeMem (cmap[2]);
               MEM_FreeMem (cmap[1]);
               MEM_FreeMem (cmap[0]);
            
            }
            
            MEM_FreeMem (pColorsUsed);
         }
      }
      
      /*
      ** Write output palette 
      */
      WriteAnyPaletteFile (ARG(OutFile), EntriesTotal, kindpal, pinvcmap, ppcondataNew);
      
      
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
                             SplicePalettes                              
 *************************************************************************

   SYNOPSIS
		BOOL SplicePalettes (LST_LIST *plistpalrec, PCONDATA *ppcondataNew)

   PURPOSE
  		
  
   INPUT
		plistpalrec  :
		ppcondataNew :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		03/19/97 : Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

BOOL SplicePalettes (LST_LIST *plistpalrec, PCONDATA *ppcondataNew)
BEGINFUNC (SplicePalettes)
{
	BOOL fSuccess;
   PALREC *ppalrec;
   MERGE armerge[ENTRIES_MAX];
   
   fSuccess = TRUE;
   
   memset (armerge, 0, sizeof (armerge));
   for (ppalrec = (PALREC *)LST_Head (plistpalrec); !LST_IsEOList(ppalrec); ppalrec = (PALREC *)LST_Next(ppalrec))
   {
      int i;
      PCONDATA *ppcon;
      MERGE *pmerge;
      
      for (
         i = 0, 
            ppcon = ppalrec->ppcondata, 
            pmerge = armerge;
         i < ppalrec->NumInPalette; 
         i++, ppcon++, pmerge++
      )
      {
         int code;
         
         code = ppcon->Constraint & (GFF_PCON_NOT_SETTABLE | GFF_PCON_NOT_USABLE);
                      
         switch (code) {
         case codeOpen:  /* Open: settable and usable */
            if (mkSet == pmerge->mergekind)
            {
               if ((pmerge->pcon.Red != ppcon->Red || pmerge->pcon.Green != ppcon->Green || pmerge->pcon.Blue != ppcon->Blue))
               {
                  ErrMess("'%s' and '%s' set index %d differently.\n", LST_NodeName(ppalrec), pmerge->psz,  i);
                  fSuccess = FALSE;
               }
               else
               {
                  WarnMess("'%s' and '%s' both set index %d to same color.\n", LST_NodeName(ppalrec), pmerge->psz,  i);
               }
            }
            else if (mkConstRef == pmerge->mergekind && 
               (pmerge->pcon.Red != ppcon->Red || pmerge->pcon.Green != ppcon->Green || pmerge->pcon.Blue != ppcon->Blue))
            {
               ErrMess("'%s' sets color unexpected by '%s' at %d.\n", LST_NodeName(ppalrec), pmerge->psz,  i);
               fSuccess = FALSE;
            }
            else
            {
               pmerge->pcon = *ppcon;
               pmerge->psz = LST_NodeName(ppalrec);
               pmerge->mergekind = mkSet;
            }
            break;
         case codeConst:  /* Constant: not settable but usable */
            /* color should match same place accross all palettes */
            if (mkSet == pmerge->mergekind &&
               (pmerge->pcon.Red != ppcon->Red || pmerge->pcon.Green != ppcon->Green || pmerge->pcon.Blue != ppcon->Blue))
            {
               ErrMess("'%s' expects color that '%s' set differently at index %d.\n", LST_NodeName(ppalrec), pmerge->psz,  i);
               fSuccess = FALSE;
            }
            else if (mkConstRef == pmerge->mergekind && 
               (pmerge->pcon.Red != ppcon->Red || pmerge->pcon.Green != ppcon->Green || pmerge->pcon.Blue != ppcon->Blue))
            {
               ErrMess("'%s' and '%s' have different expectation at index %d.\n", LST_NodeName(ppalrec), pmerge->psz,  i);
               fSuccess = FALSE;
            }
            else 
            {
               pmerge->pcon = *ppcon;
               pmerge->psz = LST_NodeName(ppalrec);
               pmerge->mergekind = mkConstRef;
            }
            break;
         case codeBarren:  /* Barren: settable but not usable */
            /* Should never occur */
            ENSURE(0);
            break;
         case codeBlocked:  /* Blocked: not settable and not usable */
            break;
         default:
            ENSURE (0);
            break;
         }
      }
   }
   if (fSuccess)
   {
      int i;
      PCONDATA *ppconNew;
      MERGE *pmerge;
      
      
      for (
         i = 0, 
            ppconNew = ppcondataNew,
            pmerge = armerge;
         i < ENTRIES_MAX;
         i++, ppconNew++, pmerge++
      )
      {
         *ppconNew = pmerge->pcon;
         ppconNew->Constraint = (uint8)((mkOpen == pmerge->mergekind) ? 0 : GFF_PCON_NOT_SETTABLE);
      }
   }

	RETURN fSuccess;
} ENDFUNC (SplicePalettes)

/*************************************************************************
                           WriteAnyPaletteFile                           
 *************************************************************************

   SYNOPSIS
		BOOL WriteAnyPaletteFile (
         char *pszOutFile, 
         int TotalColors, 
         KINDPAL kindpal,
         UINT8 *pinvcmap,
         PCONDATA *ppcondata
		)

   PURPOSE
  		To write whatever kind of palette file is indicated by kindpal
  		
  
   INPUT
		pszOutFile  : File name to use for file output
		TotalColors :
		kindpal     :
      pinvcmap    : Inverse color map.
      ppcondata   : 
  
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
   int TotalColors, 
   KINDPAL kindpal,
   UINT8 *pinvcmap,
   PCONDATA *ppcondata
)
BEGINFUNC (WriteAnyPaletteFile)
{
	BOOL fSuccess;
   
   fSuccess = TRUE;
   switch (kindpal) {
   case kpNone:
      break;
   case kpGFF:
      fSuccess = WritePaletteFile (pszOutFile, TotalColors, pinvcmap, ppcondata);
      break;
   case kpRGB:      
      ENSURE_(0, "RBG output Not implemented");//CHK_Write (fh, pColorMap, (TotalColors * 3));
      #if 0
      {
         int fh;
         
         fh = CHK_WriteOpen (pszOutFile);
         CHK_Write (fh, pColorMap, (TotalColors * 3));
         CHK_Close (fh);
      }
      #endif
      break;   
   case kpDirectX:      
      ENSURE_(0, "DirectX output Not implemented");//CHK_Write (fh, pColorMap, (TotalColors * 3));
      #if 0
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
      #endif
      break;
   case kpKludge:
      ENSURE_(0, "Kludge output Not implemented");//CHK_Write (fh, pColorMap, (TotalColors * 3));
      #if 0
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
      #endif
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
         int TotalColors,
         UINT8 *pinvcmap,
         PCONDATA *ppcondata
      )

   PURPOSE
  		To write out a GFF file with just a palette.
  
   INPUT
		pszOutFile  :
		TotalColors :
      pinvcmap    : Inverse color map.
      ppcondata   :
  
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
   int TotalColors,
   UINT8 *pinvcmap,
   PCONDATA *ppcondata
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
   memcpy (&pchunkpcon->Data, ppcondata, TotalColors * sizeof(PCONDATA));
   
   /* Make INVP Chunk */
   if (pinvcmap)
   {
      pchunknode = CreateGFFChunkNodeNoFail (IDINVP, HIST_CELLS);
      LST_AddTail (pgff->plistChunkNodes, pchunknode);
      pchunkinvp = (CHUNKINVP *) pchunknode->pchunk;
      memcpy (&pchunkinvp->Data, pinvcmap, HIST_CELLS);
   }


   
   WriteGFF(pszOutFile, pgff);
   FreeGFF (pgff);
   
	RETURN fSuccess;
} ENDFUNC (WritePaletteFile)

/*************************************************************************
                              ReadAPalette                               
 *************************************************************************

   SYNOPSIS
		UINT8 *ReadAPalette (
         PALREC   *ppalrec
		)

   PURPOSE
  		Read a palette from a file
  
   INPUT
		ppalrec :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
      NULL on failure, else ppalrec  
  
   SEE ALSO
  
  
   HISTORY
		02/28/97 : Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

PALREC *ReadAPalette (
   PALREC   *ppalrec
)
BEGINFUNC (ReadAPalette)
{
   char *pszPaletteFile;
   
   pszPaletteFile = LST_NodeName (&ppalrec->node);
   
   
   if (!stricmp (".gff", EIO_Ext(pszPaletteFile)))
   {
   	MEMFILE				*mf;
   
   	mf = MEMFILE_Load (pszPaletteFile);
   	if (mf)
   	{
   
         CHUNKHEADER   chunkheader;
         int result, headerresult;;
         BOOL  fGotPCON;
         
         fGotPCON = FALSE;
         
         /* Read chunks until find pcon chunk */
         while (!fGotPCON)
         {
            headerresult = MEMFILE_Read (mf, &chunkheader, sizeof (CHUNKHEADER));
            
            if (sizeof (CHUNKHEADER) != headerresult)
            {
               break;
            } 
            else if (!fGotPCON && chunkheader.id == IDPCON)
            { /* Found pcon chunk */
               
		       MEM_CallocMemNoFail(ppalrec->ppcondata, chunkheader.Size);
               result = MEMFILE_Read (mf, ppalrec->ppcondata, chunkheader.Size);
               ENSURE(result != 0);
               
               ppalrec->NumInPalette = chunkheader.Size / sizeof (PCONDATA);
               ENSURE(ppalrec->NumInPalette == ENTRIES_MAX);
               
               fGotPCON = TRUE;
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
      ENSURE(0);
      //pu8 = ReadRawPalette (pszPaletteFile, pNumInPalette);
   }

	RETURN ppalrec;
   
} ENDFUNC (ReadAPalette)
