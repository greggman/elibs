/*************************************************************************
 *                                                                       *
 *                              QUANTIZE.H                               *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION


   PROGRAMMERS


   FUNCTIONS

   TABS : 4 7

   HISTORY
		08/12/96 : Created.

 *************************************************************************/

#ifndef QUANTIZE_H
#define QUANTIZE_H
/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include <echidna\ensure.h>

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "malloc.h"
#include "dos.h"

#ifdef __cplusplus
extern "C" {
#endif
/*************************** C O N S T A N T S ***************************/


#ifndef TRUE
#define TRUE  (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

/*----------------------------------------------------------------------------*/
#define HIST_BIT   (6)
#define HIST_MAX   (1 << HIST_BIT)
#define A_STRIDE   (HIST_MAX * HIST_MAX * HIST_MAX)
#define R_STRIDE   (HIST_MAX * HIST_MAX)
#define G_STRIDE   (HIST_MAX)
#define B_STRIDE   (1)
#define HIST_SHIFT (8 - HIST_BIT)
#define HIST_CELLS (HIST_MAX * HIST_MAX * HIST_MAX * HIST_MAX)
#define HIST_ENTRY_TYPE          UINT16
#define HIST_ENTRY_TYPEMAX       UINT16MAX

/******************************* T Y P E S *******************************/

typedef enum {
   skOpen,        // palette site is     Usable and     Settable.
   skSeeded,      // palette site is     Usable but NOT Settable.
   skBarren,      // palette site is NOT Usable but     Settable.
   skBlocked,     // palette site is NOT Usable and NOT Settable.
   skRedundant,   // Quantizer did not consider seed because previous seed quantized to same position in histogram.
   skSuperfluous, // Quantizer did not need use seed because got best match without it.
   skToOpen,      // Temp value used when marking unitialized site for open with -U, leaving all others will be blocked.
} SITEKIND;
// Note: skBarren is of no use to the quantizer but is one of the cases if implemented with bit flags.
// skOpen and skSeeded are the only kinds of any use to the quantizer.


typedef struct {
   unsigned char  r;   
   unsigned char  g;   
   unsigned char  b;
   unsigned char  a;
   SITEKIND       SiteKind;      
} PALETTE_SITE;

/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/

#ifndef ASSERT
#define ASSERT(i) if (!(i)) { fprintf(stderr, "assertion failed: %s %i\n", __FILE__, __LINE__); exit(100); }
#endif

/************************** P R O T O T Y P E S **************************/


/* quantize.c */

int quantize(HIST_ENTRY_TYPE *histogram, int max_colors, UINT8 *color_map, int *num_colors,
    PALETTE_SITE *palsite, int num_pal_entries
);

/* inv_cmap.c */

void inv_cmap_2( int colors, unsigned char *colormap[3], int bits,
                 unsigned long *dist_buf, unsigned char *rgbmap );

#ifdef __cplusplus
}
#endif
#endif /* QUANTIZE_H */





