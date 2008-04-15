/*************************************************************************
 *                                                                       *
 *                              INV_CMAP.H                               *
 *                                                                       *
 *************************************************************************
 
		Copyright (c) 1996-2008, Echidna

		All rights reserved.

		Redistribution and use in source and binary forms, with or
		without modification, are permitted provided that the following
		conditions are met:

		* Redistributions of source code must retain the above copyright
		  notice, this list of conditions and the following disclaimer. 
		* Redistributions in binary form must reproduce the above copyright
		  notice, this list of conditions and the following disclaimer
		  in the documentation and/or other materials provided with the
		  distribution. 

		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
		CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
		INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
		MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
		DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
		BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
		EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
		TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
		DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
		ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
		OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
		OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
		POSSIBILITY OF SUCH DAMAGE.

 
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





