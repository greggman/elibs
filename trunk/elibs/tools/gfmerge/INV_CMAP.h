/*************************************************************************
 *                                                                       *
 *                              INV_CMAP.H                               *
 *                                                                       *
 *************************************************************************
 
                          Copyright 1997 Echidna                         
 
   DESCRIPTION
		Externs for inverse color map routines. 
 
   PROGRAMMERS
		
 
   FUNCTIONS
 
   TABS : 4 7
 
   HISTORY
		03/19/97 : Created.

 *************************************************************************/

#ifndef INV_CMAP_H
#define INV_CMAP_H
/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include <echidna\ensure.h>

#ifdef __cplusplus
extern "C" {
#endif
/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/


/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/


/************************** P R O T O T Y P E S **************************/

extern void inv_cmap_2( int colors, unsigned char *colormap[3], int bits,
                 unsigned long *dist_buf, unsigned char *rgbmap );


#ifdef __cplusplus
}
#endif
#endif /* INV_CMAP_H */





