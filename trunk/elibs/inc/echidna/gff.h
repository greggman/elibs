/*************************************************************************
 *                                                                       *
 *                                 GFF.H                                 *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION

       The purpose of the GFF format is to store intermediary stages of an 
       image during a pipeline of graphic operations performed by various 
       programs in batch style and to allow any necessary information to be 
       kept along the way so that important facts about the original image or 
       processes performed are available to each program in the pipeline.  
   
       The original design is geared towards a pipeline of tools for 
       perfoming image size reduction, and color reduction/palettization and 
       preservation of palette features like certain colors kept at certain 
       indexes or certain indexes not used, etc.  
   
      FORMAT:
   
       It is similar to IFF/ILBM except without the chunk padding 
       requirements and byte order in words requirement, and there are no 
       forms, just chunks.  Each chunk has a 4 character label followed by a 
       32 bit Size (of the data section) followed by the data section.  All 
       data words in the file must be of the same endian type (either all big 
       endian or all little endian).  There is a field in the first chunk 
       which indicates which order is used in the file.  The first chunk is 
       always the GGFF chunk.  Thereafter chunks may follow in any order.  
       GFF uses no compression on the pixel data.  
   
       GGFF Chunk:
           Always the first chunk.  Basic image information.
   
           Length   Name        Value Description    Comment
           ------   --------    -----------          ------------------------------
           4        ID          'G','G','F','F'      Grub Graphic File Format
           4        Size        8
           4        ByteOrder   0x1234ABCD           Big endian dump       12 34 AB CD
                                                     Little endian dump    CD AB 34 12
           2        w           Width in pixels
           2        h           Height in pixels
   
   
       RGBA Chunk:
           Pixel data plus alpha channel information.
   
           Length   Name        Value Description    Comment
           ------   --------    -----------          ------------------------------
           4        ID          'R','G','B','A'      Red Green Blue Alpha
           4        Size     
           Size     Data        Pixel Data. One byte per color component and alpha channel
                                in this order: Red, Green, Blue, Alpha. Pixels stored row major.
       PCON Chunk:
           Palette constraints chunk.  Contains information on palette 
           constraints to be maintained in the final output of the pipeline.  
   
           Length   Name        Value Description    Comment
           ------   --------    -----------          ------------------------------
           4        ID          'P','C','O','N'      
           4        Size     
           Size     Data
   
           The data is stored as a kind of annotated palette where each entry 
           is a special 4 byte sequence (RGBC) where the last byte is the constraint code
           and the first 3 bytes are R G B values.  The constraint code tells how
           this palette position may be used by each bit representing a flag for a
           specific constraint.
   
           Bit    Flag meaning
           ------ ------------
            0     Not Settable . (i.e. Quantizer was not allowed to set this index to suit image)
            1     Not Usable.    (i.e. Image was not allowed to reference this index)
            2     Reserved 
            3     Reserved 
            4     Reserved
            5     Reserved
            6     Reserved
            7     Transparent Index

       INVP Chunk:
           Inverse Palette Map for the palette containded in PCON chunk.
   
           Length   Name        Value Description    Comment
           ------   --------    -----------          ------------------------------
           4        ID          'I','N','V','P'      
           4        Size
           Size     Map         map[R][G][B]         'Size' indicates num elements
                                                      per dimension and hence bit resolution.
                                                      262144 = map[64][64][64] = 6 bits/component           
                                                       32768 = map[32][32][32] = 5 bits/component          
                                                      Etc. 

                                                                                                 
   PROGRAMMERS


   FUNCTIONS

   TABS : 4 7

   HISTORY
		08/04/96 : Created.

 *************************************************************************/

#ifndef GFF_H
#define GFF_H
/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include "echidna/memfile.h"
#include "echidna/readgfx.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************** C O N S T A N T S ***************************/

/* PCON chunk palette constraint bit flags */
#define GFF_PCON_NOT_SETTABLE    (1 << 0)
#define GFF_PCON_NOT_USABLE      (1 << 1)
#define GFF_PCON_RESERVED1       (1 << 2)
#define GFF_PCON_RESERVED2       (1 << 3)
#define GFF_PCON_RESERVED3       (1 << 4)
#define GFF_PCON_RESERVED4       (1 << 5)
#define GFF_PCON_RESERVED5       (1 << 6)
#define GFF_PCON_TRANSPARENT     (1 << 7)
#if 0
#define PCON_CANT_USE      (1 << 0)
#define PCON_CONSTANT      (1 << 1)
#define PCON_TRANSPARENT   (1 << 7)
#endif

#define U32OF4CHARS(a,b,c,d)  ( ( ((UINT32)(a)) << 24) | \
                                ( ((UINT32)(b)) << 16) | \
                                ( ((UINT32)(c)) <<  8) | \
                                ( ((UINT32)(d)) <<  0) )

#define IDOF4CHARS(a,b,c,d) (IDTYPE)(NativeToMSBF32Bit(U32OF4CHARS(a,b,c,d)))

#define IDGGFF IDOF4CHARS('G','G', 'F', 'F')
#define IDRGBA IDOF4CHARS('R','G', 'B', 'A')
#define IDPCON IDOF4CHARS('P','C', 'O', 'N')
#define IDPNDX IDOF4CHARS('P','N', 'D', 'X')
#define IDINVP IDOF4CHARS('I','N', 'V', 'P')

#define GFF_BYTE_ORDER  0x1234ABCD

/******************************* T Y P E S *******************************/
typedef UINT32 IDTYPE;
typedef struct {
   IDTYPE id;
   UINT32 Size;
} CHUNKHEADER;

typedef struct {
   UINT32   ByteOrder;  
   UINT16   Width;
   UINT16   Height;
} GGFFDATA;

typedef struct {
   UINT8 Red;
   UINT8 Green;
   UINT8 Blue;
   UINT8 Alpha;
} RGBADATA;


typedef struct {
   UINT8 Red;
   UINT8 Green;
   UINT8 Blue;
   UINT8 Constraint;
} PCONDATA;


typedef struct {
   UINT8 ColorIndex;
} INVPDATA;

typedef struct {
   CHUNKHEADER   Header;
   UINT8    u8First;    // First byte of data.
} CHUNKGENERIC;

typedef struct {
   CHUNKHEADER Header;
   GGFFDATA    Data;
} CHUNKGGFF;

typedef struct {
   CHUNKHEADER Header;
   RGBADATA    Data;
} CHUNKRGBA;

/* Pixel indexes */
typedef struct {
   CHUNKHEADER Header;
   UINT8       Data;
} CHUNKPNDX;

typedef struct {
   CHUNKHEADER Header;
   PCONDATA    Data;
} CHUNKPCON;

typedef struct {
   CHUNKHEADER Header;
   INVPDATA    Data;
} CHUNKINVP;

typedef struct {
   LST_NODE    node;
   CHUNKGENERIC *pchunk;
} CHUNKNODE;

typedef struct {
      LST_LIST listChunkNodes;
      LST_LIST *plistChunkNodes;

      // Convenience pointers:
      CHUNKGGFF *pchunkggff;    // Pointer to GGFF Chunk if present.
      CHUNKRGBA *pchunkrgba;    // Pointer to RBBA Chunk if present.
      CHUNKPCON *pchunkpcon;    // Pointer to PCON Chunk if present.
      CHUNKINVP *pchunkinvp;    // Pointer to PCON Chunk if present.
} GFF;


/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/


/************************** P R O T O T Y P E S **************************/

extern GFF        *ReadGFF (char *pszFilename);
extern void       FreeGFF (GFF *pgff);
extern GFF        *CreateGFF (void);
extern GFF        *CreateGFFNoFail (void);
extern CHUNKNODE  *CreateGFFChunkNode(IDTYPE id, UINT32 DataSize);
extern CHUNKNODE  *CreateGFFChunkNodeNoFail(IDTYPE id, UINT32 DataSize);
extern void       DestroyGFFChunkNode (CHUNKNODE *pchunknode);
extern int        WriteGFF (const char *pszFilename, GFF *pgff);
extern int        loadGFF32Bit (BlockO32BitPixels *pbop, MEMFILE *mf);
extern int        saveGFF32Bit (int fh, BlockO32BitPixels *pbop); 
extern CHUNKNODE  *PChunkNodeOfId (GFF *pgff, IDTYPE id);

#ifdef __cplusplus
}
#endif
#endif /* GFF_H */
