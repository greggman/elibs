/*************************************************************************
 *                                                                       *
 *                               READGFF.C                               *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION
      Routines to read GFF file into BlockO32BitPixels struct and write
      BlockO32BitPixels into a GFF file.

   PROGRAMMERS
      Juan M. Alvarado

   FUNCTIONS

   TABS : 4 7

   HISTORY
		08/04/96 : JMA Created.

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include "echidna/eio.h"
#include "echidna/memsafe.h"
#include "echidna/readgfx.h"
#include "echidna/checkglu.h"
#include "echidna/eerrors.h"
#include "echidna/memfile.h"
#include "echidna/gff.h"
#include "echidna/listapi.h"

/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/


/************************** P R O T O T Y P E S **************************/

void CHK_Write16Bit (int fh, UINT16 u16);
void CHK_Write32Bit (int fh, UINT32 u32);

/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/


/**************************** R O U T I N E S ****************************/

/*************************************************************************
                             PChunkNodeOfId
 *************************************************************************

   SYNOPSIS
		CHUNKNODE *PChunkNodeOfId (GFF *pgff, IDTYPE id)

   PURPOSE
      To find the chunknode of the chunk with the give id.

   INPUT
		pgff :   Pointer to GFF struct to search.
		id   :   id to find.

   OUTPUT
		None

   EFFECTS
		None

   RETURNS
      Pointer to chunknode on success. NULL on failure.

   SEE ALSO


   HISTORY
		08/05/96 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

CHUNKNODE *PChunkNodeOfId (GFF *pgff, IDTYPE id)
BEGINFUNC (PChunkNodeOfId)
{
   CHUNKNODE   *pchunknode;

   for (pchunknode = (CHUNKNODE *)LST_Head(pgff->plistChunkNodes);
         !LST_IsEOList(pchunknode);
         pchunknode = (CHUNKNODE *)LST_Next(pchunknode))
   {
      if (pchunknode->pchunk->Header.id == id)
      {
         RETURN pchunknode;  
      }
   }
	RETURN NULL;
} ENDFUNC (PChunkNodeOfId)

/*************************************************************************
                              loadGFF32Bit
 *************************************************************************

   SYNOPSIS
		int loadGFF32Bit (BlockO32BitPixels *pbop, MEMFILE *mf)

   PURPOSE
      Fill out BlockO32BitPixels struct from GFF file in memory.

   INPUT
		pbop :   Pointer to BlockO32BitPixels struct to fill out.
		mf   :   Memory file pointer.

   OUTPUT
		None

   EFFECTS
		None

   RETURNS
      TRUE on success. FALSE on failure.


   SEE ALSO
      

   HISTORY
		08/04/96 : JMA Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int loadGFF32Bit (BlockO32BitPixels *pbop, MEMFILE *mf)
BEGINFUNC (loadGFF32Bit)
{
   BOOL fSuccess;
   BOOL fFoundGGFF, fFoundRGBA, fFoundPNDX, fFoundPCON;
   UINT8 *ppndx;
   PCONDATA *ppcon;
   RGBADATA *prgba;

   pbop->rgba = NULL;

   fSuccess = FALSE;
   fFoundGGFF = FALSE;
   fFoundRGBA = FALSE;
   fFoundPNDX = FALSE;
   fFoundPCON = FALSE;

   while (!fFoundGGFF || !(fFoundRGBA || (fFoundPNDX && fFoundPCON)))
   {
      long len;
      CHUNKHEADER chunkheader; 
      
      // Read Header
      len = MEMFILE_Read (mf, &chunkheader, sizeof (CHUNKHEADER));
      if (len < sizeof(CHUNKHEADER)) break;

      if (chunkheader.id == IDGGFF)
      {
         GGFFDATA ggffdata;
         ENSURE_F (!fFoundGGFF, ("Multiple GGFF chunks in file"));
         MEMFILE_Read (mf, &ggffdata, sizeof (ggffdata));
         pbop->width = ggffdata.Width;
         pbop->height = ggffdata.Height;
         pbop->channels = 4;
         fFoundGGFF = TRUE;
      }
      else if (chunkheader.id == IDRGBA)
      {
         //printf("Found RGBA\n");
         ENSURE_F (!fFoundRGBA, ("Multiple RGBA chunks in file"));
         prgba = (RGBADATA *)mf->curPtr;
         fFoundRGBA = TRUE;
      }
      else if (chunkheader.id == IDPNDX)
      {
         //printf("Found PNDX\n");
         ENSURE_F (!fFoundPNDX, ("Multiple PNDX chunks in file"));
         ppndx = (UINT8 *)mf->curPtr;
         fFoundPNDX = TRUE;
      }
      else if (chunkheader.id == IDPCON)
      {
         //printf("Found PCON\n");
         ENSURE_F (!fFoundPCON, ("Multiple PCON chunks in file"));
         ppcon = (PCONDATA *)mf->curPtr;
         fFoundPCON = TRUE;
      }

   } // while

   fSuccess = (fFoundGGFF && fFoundRGBA || (fFoundPNDX && fFoundPCON));
   if (fSuccess)
   {
      UINT32   Size;
      // Allocate space for image.
      Size = pbop->width * pbop->height * sizeof (pixel32);
      //MEM_AllocMemNoFail (pbop->rgba, Size);
      pbop->rgba = (pixel32 *) malloc (Size);
   	if (!pbop->rgba)
   	{
   		SetGlobalErr (ERR_GENERIC);
   		GEcatf ("Out of memory reading gff");
   		return FALSE;
   	}
      
      if (fFoundRGBA)
      {
         int i;
         pixel32 *p32;

         for (
            i = pbop->width * pbop->height,
               p32 = pbop->rgba;
            i;
            i--, p32++, prgba++
         )
         {
            p32->red   = prgba->Red;
            p32->green = prgba->Green;
            p32->blue  = prgba->Blue;
            p32->alpha = prgba->Alpha;
         }
      }
      else
      {
         int i;
         pixel32 *p32;

         for (
            i = pbop->width * pbop->height,
               p32 = pbop->rgba;
            i;
            i--, p32++, ppndx++
         )
         {
            PCONDATA *ppconEntry;
            ppconEntry = ppcon + *ppndx;
            p32->red =   ppconEntry->Red;
            p32->green =   ppconEntry->Green;
            p32->blue =   ppconEntry->Blue;
            p32->alpha = (ppconEntry->Constraint & GFF_PCON_TRANSPARENT) ? 0 : 0xFF;
         }
      }
   }

	RETURN (fSuccess);

} ENDFUNC (loadGFF32Bit)

/*************************************************************************
                              saveGFF32Bit
 *************************************************************************

   SYNOPSIS
		int saveGFF32Bit (int fh, BlockO32BitPixels *pbop)

   PURPOSE
      To save an image to a GFF file given a BlockO32BitPixels struct.

   INPUT
		fh   :   File handle to save to.
		pbop :   pointer to data.

   OUTPUT
		None

   EFFECTS
		None

   RETURNS
      TRUE on success. FALSE on failure.

   SEE ALSO


   HISTORY
		08/04/96 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int saveGFF32Bit (int fh, BlockO32BitPixels *pbop)
BEGINFUNC (saveGFF32Bit)
{

   // Write GGFF Chunk
   {
      static UINT8 arID[4] = {(UINT8)'G',(UINT8)'G',(UINT8)'F',(UINT8)'F'};
      CHK_Write (fh, arID, 4);                // Write ID
      CHK_Write32Bit (fh, (UINT32)8);                  // Write Size
      CHK_Write32Bit (fh, (UINT32)0x1234ABCD);     // Write byte order code.
      CHK_Write16Bit (fh, (UINT16)pbop->width);
      CHK_Write16Bit (fh, (UINT16)pbop->height);
   }
   // Write RGBA Chunk
   {
      static UINT8 arID[4] = {(UINT8)'R',(UINT8)'G',(UINT8)'B',(UINT8)'A'};
      UINT32 Dim, Size;
      pixel32 *pp32;

      CHK_Write (fh, arID, 4);                // Write ID
      Dim = pbop->width * pbop->height;  
      Size = Dim * 4;
      CHK_Write32Bit (fh, Size);             // Write Size
      for ( pp32 = pbop->rgba; Dim; pp32++, Dim--)
      {
         UINT8 rgba[4];
         rgba[0] = pp32->red;
         rgba[1] = pp32->green;
         rgba[2] = pp32->blue;
         rgba[3] = pp32->alpha;
         CHK_Write (fh, rgba, 4);          // pixel data: r,g,b,a
      }
   }
	RETURN  TRUE;
} ENDFUNC (saveGFF32Bit)

/*************************************************************************
                             CHK_Write32Bit
 *************************************************************************

   SYNOPSIS
		void CHK_Write32Bit (int fh, UINT32 u32)

   PURPOSE
      To write a 32 bit word to a file.

   INPUT
		fh  : File handle.
		u32 : 32 bit value to write.

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		07/31/96 : JMA Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CHK_Write32Bit (int fh, UINT32 u32)
BEGINPROC (CHK_Write32Bit)
{
   CHK_Write (fh, &u32, 4);
} ENDPROC (CHK_Write32Bit)

/*************************************************************************
                             CHK_Write16Bit
 *************************************************************************

   SYNOPSIS
		void CHK_Write16Bit (int fh, UINT16 u16)

   PURPOSE
      To write a 16 bit word to a file.

   INPUT
		fh  : File handle
		u16 : 16 bit value to write.

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		07/31/96 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CHK_Write16Bit (int fh, UINT16 u16)
BEGINPROC (CHK_Write16Bit)
{
   CHK_Write (fh, &u16, 2);
} ENDPROC (CHK_Write16Bit)

/*************************************************************************
                                 PrintID
 *************************************************************************

   SYNOPSIS
		void PrintID (IDTYPE id)

   PURPOSE


   INPUT
		id :

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		08/04/96 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void PrintID (IDTYPE id)
BEGINPROC (PrintID)
{
   char *pch;
   pch = (char *)&id;
   printf ("%c%c%c%c", pch[0],pch[1], pch[2], pch[3]);

} ENDPROC (PrintID)

/*************************************************************************
                                 ReadGFF
 *************************************************************************

   SYNOPSIS
		GFF *ReadGFF (char *pszFilename)

   PURPOSE
      To read a GFF file.

   INPUT
		pszFilename : name of file

   OUTPUT
		None

   EFFECTS
		None

   RETURNS
      Pointer to allocated GFF structure on success. NULL on failure.

   SEE ALSO


   HISTORY
		08/03/96 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

GFF *ReadGFF (char *pszFilename)
BEGINFUNC (ReadGFF)
{
   GFF   *pgff;
   int   fh;

   pgff = CreateGFF();
   if (!pgff)
   {
      SetGlobalErr (ERR_GENERIC);
      GEcatf ("Out of memory reading gff");
      return FALSE;
   }
   
   fh = CHK_ReadOpen (pszFilename);
   if (fh)
   {
      for (;;)
      {
         long len;
         CHUNKHEADER   chunkheader;
         CHUNKNODE    *pchunknode;
         CHUNKGENERIC *pchunk;

         // Read Header
         len = EIO_Read (fh, &chunkheader, sizeof (CHUNKHEADER));
         if (len <= 0 ) break;

         // Create new chunk node if was able to read header
         pchunknode = CreateGFFChunkNode (chunkheader.id, chunkheader.Size);
         pchunk = pchunknode->pchunk;

         #if 0
         PrintID (pchunk->Header.id);
         printf(": %d\n", pchunk->Header.Size);
         #endif

         // Read Data
         CHK_Read (fh, &pchunk->u8First, chunkheader.Size);

         // Fill in convenience pointer for this chunk if necessary.
         if (pchunk->Header.id == IDGGFF)
         {
            //printf("Found GGFF\n");
            ENSURE_F (NULL == pgff->pchunkggff, ("Multiple GGFF chunks in file"));
            pgff->pchunkggff = (CHUNKGGFF *)pchunk;  
         }
         else if (pchunk->Header.id == IDRGBA)
         {
            //printf("Found RGBA\n");
            ENSURE_F (NULL == pgff->pchunkrgba, ("Multiple RGBA chunks in file"));
            pgff->pchunkrgba = (CHUNKRGBA *)pchunk;  
         }
         else if (pchunk->Header.id == IDPCON)
         {
            //printf("Found PCON\n");
            ENSURE_F (NULL == pgff->pchunkpcon, ("Multiple PCON chunks in file"));
            pgff->pchunkpcon = (CHUNKPCON *)pchunk;  
         }
         else if (pchunk->Header.id == IDINVP)
         {
            //printf("Found INVP\n");
            ENSURE_F (NULL == pgff->pchunkinvp, ("Multiple INVP chunks in file"));
            pgff->pchunkinvp = (CHUNKINVP *)pchunk;  
         }
 
         // Add new chunk node to list
         LST_AddTail (pgff->plistChunkNodes, pchunknode);

      } // for(;;)

      CHK_Close(fh);
   }

	RETURN pgff;
} ENDFUNC (ReadGFF)

/*************************************************************************
                                CreateGFF                                
 *************************************************************************

   SYNOPSIS
		GFF *CreateGFF (void)

   PURPOSE
  		Allocate a gff stuct and return pointer. If you use this
      and CreateGFFChunkNode () to build your GFF struct then you 
      can use FreeGFF to get rid of it at the end.
  
   INPUT
		None
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		08/30/96 : Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

GFF *CreateGFF (void)
BEGINFUNC (CreateGFF)
{
   GFF   *pgff;
   
   /*
   ** Init list of chunk nodes
   */
   //MEM_CallocMemNoFail (pgff, sizeof (GFF));
   pgff = (GFF *) calloc (1, sizeof(GFF));
   if (!pgff)
   {
      SetGlobalErr (ERR_GENERIC);
      GEcatf ("Out of memory reading gff");
      return FALSE;
   }
   
   LST_InitList(&pgff->listChunkNodes);
   pgff->plistChunkNodes = &pgff->listChunkNodes;


	RETURN pgff;
} ENDFUNC (CreateGFF)

/*************************************************************************
                             CreateGFFNoFail                             
 *************************************************************************

   SYNOPSIS
		GFF *CreateGFFNoFail (void)

   PURPOSE
  		Same as CreateGFF but cause program exits on out of memory error.
  
   INPUT
		None
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		08/30/96 : Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

GFF *CreateGFFNoFail (void)
BEGINFUNC (CreateGFFNoFail)
{
   GFF *pgff;
   
   pgff = CreateGFF ();
   if (!pgff)
   {
		exit (EXIT_FAILURE);
   }

	RETURN pgff;
} ENDFUNC (CreateGFFNoFail)

/*************************************************************************
                           CreateGFFChunkNode
 *************************************************************************

   SYNOPSIS
		CHUNKNODE *CreateGFFChunkNode (IDTYPE id, UINT32 DataSize)

   PURPOSE
      To allocate memory for a gff chunk node including chunk struct and data.

   INPUT
      id       : Id type if this new chunk.
		DataSize : Size of data in this chunk

   OUTPUT
		None

   EFFECTS
		None

   RETURNS


   SEE ALSO


   HISTORY
		08/12/96 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

CHUNKNODE *CreateGFFChunkNode (IDTYPE id, UINT32 DataSize)
BEGINFUNC (CreateGFFChunkNode)
{
   CHUNKGENERIC *pchunk;
   CHUNKNODE *pchunknode;

   pchunknode = (CHUNKNODE *)CHK_CreateNode2(sizeof(CHUNKNODE), NULL, "chunk");
   //MEM_AllocMemNoFail (pchunk, (sizeof (CHUNKHEADER) +  DataSize));
   pchunk = (CHUNKGENERIC *)malloc ((sizeof (CHUNKHEADER) +  DataSize));
   if (!pchunk)
   {
      SetGlobalErr (ERR_GENERIC);
      GEcatf ("Out of memory reading gff");
      return FALSE;
   }
   
   pchunk->Header.id = id;
   pchunk->Header.Size = DataSize;
   pchunknode->pchunk = pchunk;

	RETURN pchunknode;
} ENDFUNC (CreateGFFChunkNode)

/*************************************************************************
                        CreateGFFChunkNodeNoFail                         
 *************************************************************************

   SYNOPSIS
		CHUNKNODE *CreateGFFChunkNodeNoFail (IDTYPE id, UINT32 DataSize)

   PURPOSE
  		Same as CreateGFFChunkNode except does not return if unsuccesful, just
      causes program termination.
        
   INPUT
		id       :
		DataSize :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		08/30/96 : Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

CHUNKNODE *CreateGFFChunkNodeNoFail (IDTYPE id, UINT32 DataSize)
BEGINFUNC (CreateGFFChunkNodeNoFail)
{
   CHUNKNODE *pchunknode;
   pchunknode = CreateGFFChunkNode (id,DataSize);
   if (!pchunknode)
   {
		exit (EXIT_FAILURE);
   }
	RETURN pchunknode;
   
} ENDFUNC (CreateGFFChunkNodeNoFail)



/*************************************************************************
                            DestroyGFFChunkNode
 *************************************************************************

   SYNOPSIS
		extern void DestroyGFFChunkNode (CHUNKNODE *pchunknode)

   PURPOSE
      Free the chunk data and chunk node.

   INPUT
		pchunknode :

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		08/12/96 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

extern void DestroyGFFChunkNode (CHUNKNODE *pchunknode)
BEGINPROC (DestroyGFFChunkNode)
{
   CHUNKGENERIC *pchunk;
   
   pchunk = pchunknode->pchunk;
   //MEM_FreeMem (pchunk);
   free (pchunk);
   CHK_DeleteNode2 (pchunknode, "chunk");

} ENDPROC (DestroyGFFChunkNode)

/*************************************************************************
                                 FreeGFF
 *************************************************************************

   SYNOPSIS
		void FreeGFF (GFF *pgff)

   PURPOSE
      Free memory allocated for GFF struct.

   INPUT
		pgff :

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		08/12/96 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void FreeGFF (GFF *pgff)
BEGINPROC (FreeGFF)
{
   CHUNKNODE   *pchunknode;

   for (pchunknode = (CHUNKNODE *)LST_Head(pgff->plistChunkNodes);
         !LST_IsEOList(pchunknode);
   )
   {
      CHUNKNODE   *pchunknodeDel;  

      pchunknodeDel = pchunknode;
      pchunknode = (CHUNKNODE *)LST_Next(pchunknode);
      LST_Remove (pchunknodeDel);
      DestroyGFFChunkNode (pchunknodeDel);
   }
   //MEM_FreeMem (pgff);
   free (pgff);


} ENDPROC (FreeGFF)

/*************************************************************************
                                WriteGFF
 *************************************************************************

   SYNOPSIS
		int WriteGFF (const char *pszFilename, GFF *pgff)

   PURPOSE
      To write out a GFF structure to a file.

   INPUT
		pszFileName :
		pgff        :

   OUTPUT
		None

   EFFECTS
		None

   RETURNS


   SEE ALSO
      TRUE on success. FALSE on failure.

   HISTORY
		08/04/96 : Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int WriteGFF (const char *pszFilename, GFF *pgff)
BEGINFUNC (WriteGFF)
{
	int	fh;
   CHUNKNODE   *pchunknode;

   fh = CHK_WriteOpen (pszFilename);
   for (pchunknode = (CHUNKNODE *)LST_Head(pgff->plistChunkNodes);
         !LST_IsEOList(pchunknode);
         pchunknode = (CHUNKNODE *)LST_Next(pchunknode))
   {
      CHUNKGENERIC *pchunk;
      pchunk = pchunknode->pchunk;

      // Write Header
         CHK_Write (fh, &pchunk->Header, sizeof(CHUNKHEADER));
      // Write Data
         CHK_Write (fh, &pchunk->u8First, pchunk->Header.Size);
   }
   CHK_Close (fh);

	RETURN TRUE;
} ENDFUNC (WriteGFF)

