/*************************************************************************
 *                                                                       *
 *                               READTGA.H                               *
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

#ifndef READTGA_H
#define READTGA_H

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include "echidna/readgfx.h"
#include "echidna/memfile.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************** I N C L U D E S ****************************/


/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/


/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/


/************************** P R O T O T Y P E S **************************/

extern int loadTGA32Bit (BlockO32BitPixels *bop, MEMFILE *mf);
extern int saveTGA32Bit (int fh, BlockO32BitPixels *bop);
extern int loadTGAGrey8Bit (BlockOGrey8BitPixels *bop, MEMFILE *mf);
extern int saveTGAGrey8Bit (int fh, BlockOGrey8BitPixels *bop);

#ifdef __cplusplus
}
#endif

#endif /* READTGA_H */






