/*
 * READPIC.H
 *
 * COPYRIGHT (c) 1992 Echidna.
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 09/30/94
 *   MODIFIED : 12/22/94
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

#ifndef READPIC_H
#define READPIC_H

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include "echidna/readgfx.h"
#include "echidna/memfile.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int loadPIC32Bit( BlockO32BitPixels* bop, MEMFILE* mf);

#ifdef __cplusplus
}
#endif

#endif /* READPIC_H */

