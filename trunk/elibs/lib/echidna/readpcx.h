/*
 * PCX.H
 *
 * COPYRIGHT (c) 1992 Echidna.
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 07/30/92
 *   MODIFIED : 07/31/93
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		
 *
 * HISTORY
 *
*/

#ifndef READPCX_H
#define READPCX_H

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include "echidna/readgfx.h"
#include "echidna/memfile.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/

extern int loadPCX32Bit (BlockO32BitPixels *bop, MEMFILE *mf);
extern int loadPCX8Bit (BlockO8BitPixels *bop, MEMFILE *mf);
extern int SavePCX8Bit (int fh, BlockO8BitPixels *bop);

#ifdef __cplusplus
}
#endif

#endif /* READPCX_H */
