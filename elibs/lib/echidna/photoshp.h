/*
 * PHOTOSHP.H
 *
 *  COPYRIGHT : 1994 Echidna.
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 09/13/94
 *   MODIFIED : 09/13/94
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oO///_
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

#ifndef PHOTOSHP_H
#define PHOTOSHP_H

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

int loadPhotoshop32Bit (BlockO32BitPixels *blockPtr, MEMFILE *mf);

#ifdef __cplusplus
}
#endif

#endif /* PHOTOSHP_H */


