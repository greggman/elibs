/*
 *      A Practical Implementation of Xiaolin Wu's Color Quantizer
 *           (See Graphics Gems, Volume Two, Pages 126-133)
 *                     October 23, 1995 - v2
 *
 * Author:
 *
 * Rich Geldreich, Jr.
 *
 * Description:
 *
 * This module is an implementation of a high-speed, low-memory
 * and relatively easy to understand statistical color quantizer.
 * Its operation is similar to other Heckbert-style quantizers,
 * except that each box is weighted by variance (instead of by the
 * much more naive methods of weighting boxes by either population, size,
 * or a combination of the two), and each box is splitted on the
 * axis which will minimize the sum of the variances of both new boxes.
 *
 * Notes:
 *
 * (1) Int's are assumed to be at least 32-bits wide.
 *
 * (2) The area sum table approach to gathering color statistics is not
 * implemented here to conserve memory. Instead, a brute force method
 * of gathering color statistics is employed, which is surprisingly fast
 * and easily optimized in assembler.
 *
 * (3) A binary tree based priority list is employed to speed up the
 * search for the box with the largest variance. This differs from the
 * usual technique of using a simple linear search.
 *
 * (4) Although floating point math is used in the variance() function,
 * this may be easily replaced with high-precision fixed point math on
 * machines with weak floating point math capability.
 *
 * (5) The output of this function is an array of 8-bit palette entries.
 * It is up to you to map the original image's true-color (24-bit) pixels
 * to palettized (8-bit or less) pixels.  Spencer W. Thomas's  inverse
 * colormap functions serve this purpose very well. (See INV_CMAP.C)
 
 JMA:
   Changed ASSERTION definition and calls to use conventional meaning.
   Added calculations for preset colors.   
 TODO:   
      Figure out what to do if two seeded colors are so close that they are the same
          when quantized by the historgram
   
 *
 */
/**************************** I N C L U D E S ****************************/
/*----------------------------------------------------------------------------*/
#include "quantize.h"
//#include "quiet.h"
#include <echidna\memsafe.h>

/*************************** C O N S T A N T S ***************************/
#define USE_ORIGINAL  0
   #define rset (255 >> HIST_SHIFT)
   #define gset (255 >> HIST_SHIFT)
   #define bset (0   >> HIST_SHIFT)
   
/******************************* T Y P E S *******************************/
/*----------------------------------------------------------------------------*/

#define uchar  UINT8        
#define INT    INT32
#define uint   UINT32       
typedef unsigned long  ulong;

typedef struct
{
  uint variance;           /* weighted variance */
  uint total_weight;       /* total weight */
  uint tt_sum;             /* tt_sum += a*a+r*r+g*g+b*b*weight over entire box */
  uint t_ur;               /* t_ur += r*weight over entire box */
  uint t_ug;               /* t_ug += g*weight over entire box */
  uint t_ub;               /* t_ub += b*weight over entire box */
  uint t_ua;               /* t_ua += a*weight over entire box */
  INT  ia, ir, ig, ib;         /* upper and lower bounds */
  INT  ja, jr, jg, jb;
  
  // Variables for tracking info on boxes with seeded colors (preset colors that cannot be changed).
  int  fHasSeed;            /* Flag: Does this box have a preset quantized color? */
  INT  rSeed;              /* PreSet quantized color for this box */
  INT  gSeed;
  INT  bSeed;
  INT  aSeed;
  uint SeedVariance;  /* Total variance from preset quantized color in this box */
} box;

typedef struct {
   HIST_ENTRY_TYPE  *phist;
   HIST_ENTRY_TYPE   histValue;
} SEEDINFO;

typedef void (*PFN_BOXOP)(box *pbox);   // Box operation function pointer type


/************************** P R O T O T Y P E S **************************/

static void shrink_box(box *pbox);
static void no_shrink_box(box *pbox);
static void sum_box(box *pbox);
static void sum_box_seeded(box *pbox);  // Sum box using special histogram method that ignores
                                    // all but the 'seeded' colors in the histogram.

/***************************** G L O B A L S *****************************/
/*----------------------------------------------------------------------------*/
static PFN_BOXOP  pfnShrinkBox;  // Pointer to current box shrinking function to use.
static PFN_BOXOP  pfnSumBox;     // Pointer to current box summing function to use.
static box * *heap;     /* priority queue */
static INT heap_size;
static box *boxes;      /* box list */
static INT total_boxes; /* total boxes allocated */
static INT num_boxes;   /* num boxes in use */
static HIST_ENTRY_TYPE *hist;      /* histogram */
static int NumSeeded;               /* number of palette entries that can be used but not set */
static int NumOpen;                 /* number of palette that can be set and used */

/*
** arseedinfo[] is used in a mechanism that allows the preset colors to be 
** seeded into the quantization process without requiring another histogram to 
** be allocated.  Normally the histogram must contain only values in positions 
** for the colors being seeded.  In this scheme, the original histogram values for 
** the picture are left intact except at the positions of the seeded colors, 
** which are set to indexes (1..Num Seeded Colors) into arseedinfo[].  The 
** corresponding entries in arseedinfo[] contain copies of the original 
** historgram value that was overwritten and a pointer back to the histogram 
** entry that was overwritten.  The pointer is used as both a convenient way to 
** restore the originial value and to verify which histogram entries are 
** for seeded colors (by comparing their address to the pointer in 
** arseedinfo[] which their value indexes).  
** Note: position 0 is unused. This optimizes the condition check for seeded entries 
** in the special function sum_box_seeded(), by allowing it to short circuit on 0
** entries in the histogram (which will be true for most entries).
*/
static SEEDINFO *arseedinfo; 

 
/****************************** M A C R O S ******************************/


/**************************** R O U T I N E S ****************************/


static void no_shrink_box(box *pbox)
{
   // purposely empty.
   pbox = pbox;   // avoid compiler warning
}

/*----------------------------------------------------------------------------*/
/* Shrinks box to minimum possible size.                                      */
/*----------------------------------------------------------------------------*/
static void shrink_box(box *pbox)
#if 0
static void shrink_box(INT ir, INT ig, INT ib, INT ia,
                       INT jr, INT jg, INT jb, INT ja,
                       INT *lr, INT *lg, INT *lb, INT *la,
                       INT *hr, INT *hg, INT *hb, INT *ha)
#endif                       
{
   INT ir; INT ig; INT ib; INT ia;
   INT jr; INT jg; INT jb; INT ja;
   INT *lr; INT *lg; INT *lb; INT *la;
   INT *hr; INT *hg; INT *hb; INT *ha;

  INT r, g, b, a;
  HIST_ENTRY_TYPE *rp, *gp, *bp, *ap, *s;

   ir = pbox->ir;
   ig = pbox->ig;
   ib = pbox->ib;
   ia = pbox->ia;
   
   jr = pbox->jr;
   jg = pbox->jg;
   jb = pbox->jb;
   ja = pbox->ja;
   
   lr = &pbox->ir;
   lg = &pbox->ig;
   lb = &pbox->ib;
   la = &pbox->ia;
   
   hr = &pbox->jr;
   hg = &pbox->jg;
   hb = &pbox->jb;
   ha = &pbox->ja;
   
  s = hist + (ia * A_STRIDE + ir * R_STRIDE + ig * G_STRIDE + ib * B_STRIDE);

  rp = s;
  for (r = ir; r <= jr; r++)
  {
    gp = rp;
    for (g = ig; g <= jg; g++)
    {
      bp = gp;
      for (b = ib; b <= jb; b++)
      {
         ap = bp;
         for (a = ia; a <= ja; a++)
         {
           if (*ap) { *lr = r; goto lr_done; }
           ap += A_STRIDE;
         }            
         bp += B_STRIDE;
       }
      gp += G_STRIDE;
    }
    rp += R_STRIDE;
  }

lr_done:

  gp = s;

  for (g = ig; g <= jg; g++)
  {
    rp = gp;
    for (r = ir; r <= jr; r++)
    {
      bp = rp;
      for (b = ib; b <= jb; b++)
      {
          ap = bp;
          for (a = ia; a <= ja; a++)
          {
            if (*ap) { *lg = g; goto lg_done; }
            ap += A_STRIDE;
          }
          bp += B_STRIDE;
      }
      rp += R_STRIDE;
    }
    gp += G_STRIDE;
  }

lg_done:

  bp = s;

  for (b = ib; b <= jb; b++)
  {
    rp = bp;
    for (r = ir; r <= jr; r++)
    {
      gp = rp;
      for (g = ig; g <= jg; g++)
      {
        ap = gp;
        for (a = ia; a <= ja; a++)
        {
          if (*ap) { *lb = b; goto lb_done; }
          ap += A_STRIDE;
        }
        gp += G_STRIDE;
      }
      rp += R_STRIDE;
    }
    bp += B_STRIDE;
  }

lb_done:

  ap = s;

  for (a = ia; a <= ja; a++)
  {
    bp = ap;
    for (b = ib; b <= jb; b++)
    {
      rp = bp;
      for (r = ir; r <= jr; r++)
      {
        gp = rp;
        for (g = ig; g <= jg; g++)
        {
          if (*gp) { *la = a; goto la_done; }
          gp += G_STRIDE;
        }
        rp += R_STRIDE;
      }
      bp += G_STRIDE;
    }
    ap += A_STRIDE;
  }

la_done:

  s = hist + (ja * A_STRIDE + jr * R_STRIDE + jg * G_STRIDE + jb * B_STRIDE);

  rp = s;

  for (r = jr; r >= ir; r--)
  {
    gp = rp;
    for (g = jg; g >= ig; g--)
    {
      bp = gp;
      for (b = jb; b >= ib; b--)
      {
         ap = bp;
         for (a = ja; a >= ia; a--)
         {
            if (*ap) { *hr = r; goto hr_done; }
            ap -= A_STRIDE;
         }
         bp -= B_STRIDE;
      }
      gp -= G_STRIDE;
    }
    rp -= R_STRIDE;
  }

hr_done:

  gp = s;

  for (g = jg; g >= ig; g--)
  {
    rp = gp;
    for (r = jr; r >= ir; r--)
    {
      bp = rp;
      for (b = jb; b >= ib; b--)
      {
        ap = bp;
        for (a = ja; a >= ia; a--)
        {
          if (*ap) { *hg = g; goto hg_done; }
          ap -= A_STRIDE;
        }
        bp -= B_STRIDE;
      }
      rp -= R_STRIDE;
    }
    gp -= G_STRIDE;
  }

hg_done:

  bp = s;

  for (b = jb; b >= ib; b--)
  {
    gp = bp;
    for (g = jg; g >= ig; g--)
    {
      rp = gp;
      for (r = jr; r >= ir; r--)
      {
        ap = bp;
        for (a = ja; a >= ia; a--)
        {
          if (*ap) { *hb = b; goto hb_done; }
          ap -= A_STRIDE;
        }
        rp -= R_STRIDE;
      }
      gp -= G_STRIDE;
    }
    bp -= B_STRIDE;
  }

hb_done:

  ap = s;
  for (a = ja; a >= ia; a--)
  {
    bp = ap;
    for (b = jb; b >= ib; b--)
    {
      gp = bp;
      for (g = jg; g >= ig; g--)
      {
        rp = gp;
        for (r = jr; r >= ir; r--)
        {
          if (*rp) { *hb = b; goto ha_done; }
          rp -= R_STRIDE;
        }
      gp -= G_STRIDE;
      }
      bp -= B_STRIDE;
    }
    ap -= A_STRIDE;
  }
ha_done:

  return;
}
/*----------------------------------------------------------------------------*/
/* Standard binary tree based priorty queue manipulation functions.           */
/*----------------------------------------------------------------------------*/
static void down_heap(void)
{
  uint i, j, q;
  box *p;

  p = heap[1];
  q = p->variance;

  for (i = 1; ; )
  {
    if ((j = i << 1) > (uint)heap_size)
      break;

    if (j < (uint)heap_size)
    {
      if (heap[j]->variance < heap[j + 1]->variance)
        j++;
    }

    if (q >= heap[j]->variance)
      break;

    heap[i] = heap[j];

    i = j;
  }

  heap[i] = p;
}
/*----------------------------------------------------------------------------*/
static void insert_heap(box *p)
{
  uint i, j, q;

  q = p->variance;
  j = ++heap_size;

  for ( ; ; )
  {
    if ((i = j >> 1) == 0)
      break;

    if (heap[i]->variance >= q)
      break;

    heap[j] = heap[i];

    j = i;
  }

  heap[j] = p;
}
/*----------------------------------------------------------------------------*/
/* Returns "worst" box, or NULL if no more splittable boxes remain. The worst */
/* box is the box with the largest variance.                                  */
/*----------------------------------------------------------------------------*/
static box *worst_box(void)
{
   if (heap_size == 0)
      return NULL;
   else 
   {
      box *pbox;
      pbox = heap[1];
      if (!pbox->variance)
      {
         pbox = NULL;  
         // EL_printf (("Warning: Worst box has zero variance!\n"));
      }
      
      return pbox;
   }
}

/*----------------------------------------------------------------------------*/
/* Calculate statistics over the specified box. This is an implementation of  */
/* the "brute force" method of gathering statistics described earlier.        */
/*----------------------------------------------------------------------------*/
static void sum_box(box *pbox)
{
  INT i, j, r, g, b;
  uint rs, ts;
  uint w, tr, tg, tb;
  HIST_ENTRY_TYPE *rp, *gp, *bp;


  j = 0;

  tr = tg = tb = i = 0;
   if (pbox->fHasSeed)
   {
      pbox->SeedVariance = 0;
   }

  rp = hist + ((pbox->ir * R_STRIDE) + (pbox->ig * G_STRIDE) + pbox->ib);

  for (r = pbox->ir; r <= pbox->jr; r++)
  {
    rs = r * r;
    gp = rp;

    for (g = pbox->ig; g <= pbox->jg; g++)
    {
      ts = rs + (g * g);
      bp = gp;

      for (b = pbox->ib; b <= pbox->jb; b++)
      {
        if (*bp++)                /* was this cell used at all? */
        {
          w   = *(bp - 1);        /* update statistics */
          j  += w;
          tr += r * w;
          tg += g * w;
          tb += b * w;
          i  += (ts + b * b) * w;
          if (pbox->fHasSeed)
          {
            INT rdiff, gdiff, bdiff;
            rdiff = (r - pbox->rSeed);
            gdiff = (g - pbox->gSeed);
            bdiff = (b - pbox->bSeed);
            pbox->SeedVariance += w * ((uint)(rdiff * rdiff) + (uint)(gdiff * gdiff) + (uint)(bdiff * bdiff));
          }
        }
      }
      gp += G_STRIDE;
    }

    rp += R_STRIDE;
  }

  pbox->total_weight = j;
  pbox->tt_sum       = i;
  pbox->t_ur         = tr;
  pbox->t_ug         = tg;
  pbox->t_ub         = tb;

} // sum_box

static void sum_box_seeded (box *pbox)
{
  INT i, j, r, g, b;
  uint rs, ts;
  uint w, tr, tg, tb;
  HIST_ENTRY_TYPE *rp, *gp, *bp;


  j = 0;

  tr = tg = tb = i = 0;

  rp = hist + ((pbox->ir * R_STRIDE) + (pbox->ig * G_STRIDE) + pbox->ib);

  for (r = pbox->ir; r <= pbox->jr; r++)
  {
    rs = r * r;
    gp = rp;

    for (g = pbox->ig; g <= pbox->jg; g++)
    {
      ts = rs + (g * g);
      bp = gp;

      for (b = pbox->ib; b <= pbox->jb; b++, bp++)
      {
        w = 1; // Give weight to every position so nice boxes are made around seeded colors.
        if (0 < *bp && *bp <= NumSeeded && arseedinfo[*bp].phist == bp)                /* was this cell used at all? */
        {
           /* is a seeded color */
          w   = HIST_ENTRY_TYPEMAX;  // give it some big weight.
          pbox->rSeed = r;
          pbox->gSeed = g;
          pbox->bSeed = b;
        }
          j  += w;
          tr += r * w;
          tg += g * w;
          tb += b * w;
          i  += (ts + b * b) * w;
      }
      gp += G_STRIDE;
    }

    rp += R_STRIDE;
  }

  pbox->total_weight = j;
  pbox->tt_sum       = i;
  pbox->t_ur         = tr;
  pbox->t_ug         = tg;
  pbox->t_ub         = tb;
   
} // sum_box_seeded

/*----------------------------------------------------------------------------*/

static uint get_variance(box *pbox)
{
   uint variance;
   
   // Is set color in this box?
   if (pbox->fHasSeed)
   {
      variance =  pbox->SeedVariance;
   }
   else
   {
      double temp;
     /* the following calculations can be performed in fixed point
      * if needed - just be sure to preserve enough precision!
      */
      temp  = (double)pbox->t_ur * (double)pbox->t_ur;
      temp += (double)pbox->t_ug * (double)pbox->t_ug;
      temp += (double)pbox->t_ub * (double)pbox->t_ub;
      temp /= (double)pbox->total_weight;
      variance = ((uint)((double)pbox->tt_sum - temp));
   }
   
   return variance;
}


/*----------------------------------------------------------------------------*/
/* Splits box along the axis which will minimize the two new box's overall    */
/* variance. A search on each axis is used to locate the optimum split point. */
/*----------------------------------------------------------------------------*/
static void split_box(box *original_box)
{
   INT icase;
   box *left_box;
   box *right_box;                
   box *mid_box, mid_box_struct;
   int fHasSeed;
    uint left_variance, right_variance;
    uint left_variance_r, right_variance_r;
    uint left_variance_g, right_variance_g;
    uint left_variance_b, right_variance_b;
         
   /* Original box values  */
   uint total_weight;
   uint tt_sum, t_ur, t_ug, t_ub;
   INT  ir, ig, ib, jr, jg, jb;
   
   #if 0
   /* Left box values */
   uint total_weight1;
   uint tt_sum1, t_ur1, t_ug1, t_ub1;
   INT  ir1, ig1, ib1, jr1, jg1, jb1;
   
   /* Right box values */   
   uint total_weight2;
   uint tt_sum2, t_ur2, t_ug2, t_ub2;
   INT  ir2, ig2, ib2, jr2, jg2, jb2;
   
   /* Mid box values */   
   uint total_weight3;
   uint tt_sum3, t_ur3, t_ug3, t_ub3;
   #endif
   
   uint lowest_variance, variance_r, variance_g, variance_b;
   INT  pick_r, pick_g, pick_b;
   
   left_box = boxes + num_boxes; 
   num_boxes++;
   mid_box = &mid_box_struct;     // used for slice of box moved over from 
   right_box = original_box;       
   
   fHasSeed = right_box->fHasSeed;
   total_weight          = right_box->total_weight;
   tt_sum                = right_box->tt_sum;
   t_ur                  = right_box->t_ur;
   t_ug                  = right_box->t_ug;
   t_ub                  = right_box->t_ub;
   ir                    = right_box->ir;
   ig                    = right_box->ig;
   ib                    = right_box->ib;
   jr                    = right_box->jr;
   jg                    = right_box->jg;
   jb                    = right_box->jb;

   mid_box->fHasSeed          = right_box->fHasSeed          ;
   mid_box->rSeed             = left_box->rSeed = right_box->rSeed            ;
   mid_box->gSeed             = left_box->gSeed = right_box->gSeed             ;
   mid_box->bSeed             = left_box->bSeed = right_box->bSeed             ;
   
   
/************************************************************/
   /* left box's initial statistics */
   
   left_box->total_weight         = 0;
   left_box->tt_sum               = 0;
   left_box->t_ur                 = 0;
   left_box->t_ug                 = 0;
   left_box->t_ub                 = 0;
   left_box->fHasSeed             = FALSE;
   
   #if 0
   /* right box's initial statistics */
   
   right_box->total_weight         = total_weight;
   right_box->tt_sum               = tt_sum;
   right_box->t_ur                 = t_ur;
   right_box->t_ug                 = t_ug;
   right_box->t_ub                 = t_ub;
   #else
   /* right box's initial statistics are already set as values of original box */
   #endif
   
   
   /* Note: One useful optimization has been purposefully omitted from the
   * following loops. The variance function is always called twice per
   * iteration to calculate the new total variance. This is a waste of time
   * in the possibly common case when the new split point did not shift any
   * new points from one box into the other. A simple test can be added to
   * remove this inefficiency.
   */
   
   /* locate optimum split point on red axis */
   
   variance_r = 0xFFFFFFFF;
   
   left_box->ir = mid_box->ir = right_box->ir = ir;
   left_box->ig = mid_box->ig = right_box->ig = ig;
   left_box->ib = mid_box->ib = right_box->ib = ib;
   left_box->jr = mid_box->jr = right_box->jr = jr;
   left_box->jg = mid_box->jg = right_box->jg = jg;
   left_box->jb = mid_box->jb = right_box->jb = jb;
   
   for (
      left_box->jr = ir,
      mid_box->jr = ir, 
      right_box->ir = ir+1
      ; 
      mid_box->ir < jr
      ; 
      left_box->jr++,
      mid_box->ir++, mid_box->jr++,
      right_box->ir++
   )
   {
    uint total_variance;

    /* calculate the statistics for the area being taken
     * away from the right box and given to the left box
     */
     
    pfnSumBox (mid_box);

   #ifdef DEBUGGING
    ASSERT (mid_box->total_weight <= total_weight);
   #endif

    /* update left and right box's statistics */

    left_box->total_weight += mid_box->total_weight;
    left_box->tt_sum       += mid_box->tt_sum;
    left_box->t_ur         += mid_box->t_ur;
    left_box->t_ug         += mid_box->t_ug;
    left_box->t_ub         += mid_box->t_ub;

    right_box->total_weight -= mid_box->total_weight;
    right_box->tt_sum       -= mid_box->tt_sum;
    right_box->t_ur         -= mid_box->t_ur;
    right_box->t_ug         -= mid_box->t_ug;
    right_box->t_ub         -= mid_box->t_ub;

    if (fHasSeed)
    {
       left_box->SeedVariance += mid_box->SeedVariance;
       right_box->SeedVariance -= mid_box->SeedVariance;
       
       // See if set color moved from right box to left box
       if ( right_box->fHasSeed && right_box->rSeed < right_box->ir)
       {
         left_box->fHasSeed = TRUE;
         right_box->fHasSeed = FALSE;
       }
    }

   #ifdef DEBUGGING
    ASSERT ((left_box->total_weight + right_box->total_weight) == total_weight);
   #endif

    /* calculate left and right box's overall variance */

    total_variance = get_variance(left_box) + get_variance(right_box);

    /* found better split point? if so, remember it */

    if (total_variance < variance_r)
    {
      variance_r = total_variance;
      pick_r = mid_box->ir;
      left_variance_r = get_variance (left_box);
      right_variance_r = get_variance (right_box);
    }
  }
/************************************************************/

  /* left box's initial statistics */
   left_box->total_weight         = 0;
   left_box->tt_sum               = 0;
   left_box->t_ur                 = 0;
   left_box->t_ug                 = 0;
   left_box->t_ub                 = 0;
   left_box->fHasSeed             = FALSE;

  /* right box's initial statistics */

   right_box->total_weight         = total_weight;
   right_box->tt_sum               = tt_sum;
   right_box->t_ur                 = t_ur;
   right_box->t_ug                 = t_ug;
   right_box->t_ub                 = t_ub;
   right_box->fHasSeed             = fHasSeed;
   
  /* locate optimum split point on green axis */

  variance_g = 0xFFFFFFFF;

   /* restore changed box boundries */
   mid_box->ir = right_box->ir = ir;
   mid_box->jr = left_box->jr  = jr;
   
   for (
      left_box->jg = ig,
      mid_box->jg = ig, 
      right_box->ig = ig+1
      ; 
      mid_box->ig < jg
      ; 
      left_box->jg++,
      mid_box->ig++, mid_box->jg++,
      right_box->ig++
   )
   {
    uint total_variance;

    /* calculate the statistics for the area being taken
     * away from the right box and given to the left box
     */

      pfnSumBox (mid_box);

   #ifdef DEBUGGING
    ASSERT (mid_box->total_weight3 <= total_weight);
   #endif

    /* update left and right box's statistics */

    left_box->total_weight += mid_box->total_weight;
    left_box->tt_sum       += mid_box->tt_sum;
    left_box->t_ur         += mid_box->t_ur;
    left_box->t_ug         += mid_box->t_ug;
    left_box->t_ub         += mid_box->t_ub;

    right_box->total_weight -= mid_box->total_weight;
    right_box->tt_sum       -= mid_box->tt_sum;
    right_box->t_ur         -= mid_box->t_ur;
    right_box->t_ug         -= mid_box->t_ug;
    right_box->t_ub         -= mid_box->t_ub;

    if (fHasSeed)
    {
       left_box->SeedVariance += mid_box->SeedVariance;
       right_box->SeedVariance -= mid_box->SeedVariance;
       
       // See if set color moved from right box to left box
       if ( right_box->fHasSeed && right_box->gSeed < right_box->ig)
       {
         left_box->fHasSeed = TRUE;
         right_box->fHasSeed = FALSE;
       }
    }
    
   #ifdef DEBUGGING
    ASSERT ((left_box->total_weight + right_box->total_weight) == total_weight);
   #endif

    /* calculate left and right box's overall variance */

    total_variance = get_variance(left_box) + get_variance(right_box);

    /* found better split point? if so, remember it */

    if (total_variance < variance_g)
    {
      variance_g = total_variance;
      pick_g = mid_box->ig;
      left_variance_g = get_variance (left_box);
      right_variance_g = get_variance (right_box);
    }
  }

/************************************************************/
  /* left box's initial statistics */

   left_box->total_weight         = 0;
   left_box->tt_sum               = 0;
   left_box->t_ur                 = 0;
   left_box->t_ug                 = 0;
   left_box->t_ub                 = 0;
   left_box->fHasSeed             = FALSE;

  /* right box's initial statistics */

   right_box->total_weight         = total_weight;
   right_box->tt_sum               = tt_sum;
   right_box->t_ur                 = t_ur;
   right_box->t_ug                 = t_ug;
   right_box->t_ub                 = t_ub;
   right_box->fHasSeed             = fHasSeed;

  /* locate optimum split point on blue axis */
  
   variance_b = 0xFFFFFFFF;

   /* restore changed box boundries */
   mid_box->ig = right_box->ig = ig;
   mid_box->jg = left_box->jg  = jg;
   
   //for (i = ib; i < jb; i++)
   for (
      left_box->jb = ib,
      mid_box->jb = ib, 
      right_box->ib = ib+1
      ; 
      mid_box->ib < jb
      ; 
      left_box->jb++,
      mid_box->ib++, mid_box->jb++,
      right_box->ib++
   )
   {
    uint total_variance;

    /* calculate the statistics for the area being taken
     * away from the right box and given to the left box
     */

      pfnSumBox(mid_box);
    //sum(ir, ig, i, jr, jg, i,
    //    &total_weight3, &tt_sum3, &t_ur3, &t_ug3, &t_ub3);

   #ifdef DEBUGGING
    ASSERT (mid_box->total_weight <= total_weight);
   #endif

    /* update left and right box's statistics */

    left_box->total_weight += mid_box->total_weight;
    left_box->tt_sum       += mid_box->tt_sum;
    left_box->t_ur         += mid_box->t_ur;
    left_box->t_ug         += mid_box->t_ug;
    left_box->t_ub         += mid_box->t_ub;

    right_box->total_weight -= mid_box->total_weight;
    right_box->tt_sum       -= mid_box->tt_sum;
    right_box->t_ur         -= mid_box->t_ur;
    right_box->t_ug         -= mid_box->t_ug;
    right_box->t_ub         -= mid_box->t_ub;

    if (fHasSeed)
    {
       left_box->SeedVariance += mid_box->SeedVariance;
       right_box->SeedVariance -= mid_box->SeedVariance;
       
       // See if set color moved from right box to left box
       if ( right_box->fHasSeed && right_box->bSeed < right_box->ib)
       {
         left_box->fHasSeed = TRUE;
         right_box->fHasSeed = FALSE;
       }
    }
    
   #ifdef DEBUGGING
    ASSERT ((left_box->total_weight + right_box->total_weight) == total_weight);
   #endif

    /* calculate left and right box's overall variance */

    total_variance = get_variance(left_box) + get_variance (right_box);

    /* found better split point? if so, remember it */

    if (total_variance < variance_b)
    {
      variance_b = total_variance;
      pick_b = mid_box->ib;
      left_variance_b = get_variance (left_box);
      right_variance_b = get_variance (right_box);
    }
  }

/************************************************************/

   /* restore changed box boundries */
   mid_box->ib = right_box->ib = ib;
   mid_box->jb = left_box->jb  = jb;

  /* now find out which axis should be split */

  lowest_variance = variance_r;
  icase = 0;
  left_variance = left_variance_r;
  right_variance = right_variance_r;

  if (variance_g < lowest_variance)
  {
    lowest_variance = variance_g;
    icase = 1;
     left_variance = left_variance_g;
     right_variance = right_variance_g;
  }

  if (variance_b < lowest_variance)
  {
    lowest_variance = variance_b;
    icase = 2;
    left_variance = left_variance_b;
    right_variance = right_variance_b;
  }

  /* split box on the selected axis */

  left_box->ir = ir; left_box->ig = ig; left_box->ib = ib;
  right_box->jr = jr; right_box->jg = jg; right_box->jb = jb;

  switch (icase)
  {
    case 0:
    {
      left_box->jr = pick_r + 0; left_box->jg = jg; left_box->jb = jb;
      right_box->ir = pick_r + 1; right_box->ig = ig; right_box->ib = ib;
      break;
    }
    case 1:
    {
      left_box->jr = jr; left_box->jg = pick_g + 0; left_box->jb = jb;
      right_box->ir = ir; right_box->ig = pick_g + 1; right_box->ib = ib;
      break;
    }
    case 2:
    {
      left_box->jr = jr; left_box->jg = jg; left_box->jb = pick_b + 0;
      right_box->ir = ir; right_box->ig = ig; right_box->ib = pick_b + 1;
      break;
    }
  }
   
   if (fHasSeed)
   {
      // Mark box that ended up with set color.
      
      // Check if it was right box
      if (  right_box->ir <= right_box->rSeed &&  right_box->rSeed <= right_box->jr
         && right_box->ig <= right_box->gSeed &&  right_box->gSeed <= right_box->jg
         && right_box->ib <= right_box->bSeed &&  right_box->bSeed <= right_box->jb
      )
      {
         right_box->fHasSeed = TRUE;
         left_box->fHasSeed = FALSE;
      }
      else // was left box.
      {
         right_box->fHasSeed = FALSE;
         left_box->fHasSeed = TRUE;
      }
   }

  /* shrink the new boxes to their minimum possible sizes */

  pfnShrinkBox(left_box);
  pfnShrinkBox(right_box);

  /* update statistics */

   pfnSumBox (left_box);

  right_box->total_weight         = total_weight - left_box->total_weight;
  right_box->tt_sum               = tt_sum - left_box->tt_sum;
  right_box->t_ur                 = t_ur - left_box->t_ur;
  right_box->t_ug                 = t_ug - left_box->t_ug;
  right_box->t_ub                 = t_ub - left_box->t_ub;

  /* create the new boxes */
  left_box->variance = left_variance;
  right_box->variance = right_variance;
  
  /* enter all splittable boxes into the priory queue */
  
  icase = 0;
  if ((right_box->jr - right_box->ir) + (right_box->jg - right_box->ig) + (right_box->jb - right_box->ib)) icase = 2;
  if ((left_box->jr - left_box->ir) + (left_box->jg - left_box->ig) + (left_box->jb - left_box->ib)) icase++;

  switch (icase)
  {
    case 0: // Neither box is further splitable.
    {
      heap[1] = heap[heap_size]; // Overwrite old box in heap  with copy of smaller box at bottom of heap.

      heap_size--;   // shrink heap size, effectively deleting smaller box at bottom of heap.

      if (heap_size)
        down_heap(); // adjust heap to account for smaller sized box introduced at root.

      break;
    }
    case 1: // new left_box is splitable, old right_box is not.
    {
      heap[1] = left_box;  // replace old box position in heap with new box

      down_heap();   // adjust heap to account for smaller size of box that is replacing old box.

      break;
    }
    case 2: // Old right_box is splittable, new left_box is not.
    {
      down_heap();   // just adjust heap to account for reduced size of old box that was already in heap.

      break;
    }
    case 3: // Both boxes are splittable
    {
      down_heap();   // adjust heap to account for reduced size of old box that was already in heap.

      insert_heap(left_box);  // insert new box into heap.

      break;
    }
  }
  
} /* split_box */

/*----------------------------------------------------------------------------*/
/* Creates new colormap.                                                      */
/*----------------------------------------------------------------------------*/
static void make_color_map(uchar *color_map, 
    PALETTE_SITE *palsite, int num_pal_entries 
)
{
   INT i;
   int c;  // Color Table Index.
   box *p;
   
   p = boxes;

   c = 0;
   for (i = 0; i < num_boxes; i++, p++)
   {
      if (p->fHasSeed) {continue;}
      
      for ( NULL; palsite->SiteKind != skOpen; c++, color_map += 3, palsite++) {};
      
      ASSERT(c < num_pal_entries);
      
      {
         uint total_weight;
         
         total_weight = p->total_weight;
         
         *color_map = (uchar)(((p->t_ur << HIST_SHIFT) + (total_weight >> 1)) / total_weight);
         *color_map++ |= (*color_map >> (8 - HIST_SHIFT));
         
         *color_map = (uchar)(((p->t_ug << HIST_SHIFT) + (total_weight >> 1)) / total_weight);
         *color_map++ |= (*color_map >> (8 - HIST_SHIFT));
         
         *color_map = (uchar)(((p->t_ub << HIST_SHIFT) + (total_weight >> 1)) / total_weight);
         *color_map++ |= (*color_map >> (8 - HIST_SHIFT));
         
         ++c;
         ++palsite;
      }
    
   }
}
/*----------------------------------------------------------------------------*/
/* Create initial box, initialize heap.                                       */
/*----------------------------------------------------------------------------*/
static INT initialize(INT colors, 
    PALETTE_SITE *palsite, int num_pal_entries
)
{
  total_boxes = colors + NumSeeded; 

  if ((heap = malloc(sizeof(box *) * (total_boxes + 1))) == NULL)
    return TRUE;

  if ((boxes = calloc(total_boxes, sizeof(box))) == NULL)
    return TRUE;

  if (NumSeeded)
  {
     if ((arseedinfo = calloc((NumSeeded+1), sizeof(SEEDINFO))) == NULL)
       return TRUE;
      /*
      ** Prepare Seed info
      */
      {
         int i, seed;   
         PALETTE_SITE *ps;
         seed = 0;
         for (i = num_pal_entries, ps = palsite;  i;  i--, ps++)
         {
            if (skSeeded == ps->SiteKind)
            {
               int r,g,b,a;
               
               HIST_ENTRY_TYPE *ph;
               
               seed++;
               r = (ps->r >> HIST_SHIFT);
               g = (ps->g >> HIST_SHIFT);
               b = (ps->b >> HIST_SHIFT);
               a = (ps->b >> HIST_SHIFT);
               ph = hist + a * A_STRIDE +  r * R_STRIDE + g * G_STRIDE + b * B_STRIDE;
               // Check if have collision of two seeds at same histogram site */
               if (0 < *ph && *ph < seed && arseedinfo[*ph].phist == ph)
               {
                  // Collision. So skip this redundant seed.
                  seed--;
                  NumSeeded--;
                  ps->SiteKind = skRedundant; // communicate back that this seed position was not used.
               }
               else
               {
                  arseedinfo[seed].phist     = ph;
                  arseedinfo[seed].histValue = *ph;
                  *ph = seed;
               }
            }
         }
      }
  }


  boxes->ia = boxes->ir = boxes->ig = boxes->ib = 0;
  boxes->ja = boxes->jr = boxes->jg = boxes->jb = HIST_MAX - 1;

  /* shrink initial box to minimum possible size */
  shrink_box(boxes); // always call real shrink_box here. Don't go through func pointer.


  /* calculate the initial box's statistics */
  if (NumSeeded) 
  {
   sum_box_seeded (boxes); // always call original sum_box_seeded here. Don't go through func pointer.
  }
  else
  {
   sum_box (boxes); // always call original sum_box here. Don't go through func pointer.
  }

  boxes->variance     = 1;

  /* enter box into heap if it's splittable */

  num_boxes           = 1;
  heap_size           = 0;

  if ((boxes->ja - boxes->ia) + (boxes->jr - boxes->ir) + (boxes->jg - boxes->ig) + (boxes->jb - boxes->ib))
  {
    heap[1] = boxes;
    heap_size = 1;
  }

  return FALSE;
}

/*----------------------------------------------------------------------------*/
int quantize(HIST_ENTRY_TYPE *histogram, int max_colors, uchar *color_map, int *num_colors, 
    PALETTE_SITE *palsite, int num_pal_entries
)
{
   INT status = FALSE;
   box *pbox;
  
   heap  = NULL;
   boxes = NULL;
   arseedinfo = NULL;
   
   hist  = histogram;
  
   /*
   ** Count seeds and open slots and copy preset colors to color_table
   */
   {
      int i;
      PALETTE_SITE *ps;
      uchar *pcolor;
      
      NumSeeded = NumOpen = 0;
      for (i = num_pal_entries, ps = palsite, pcolor = color_map;  i;  i--, ps++)
      {
         NumOpen  += (skOpen == ps->SiteKind);
         NumSeeded += (skSeeded == ps->SiteKind);
         /*
         ** Copy unchangable colors directly into color_table. (Note: copies changable ones 
         ** too for efficiency but they will get overwritten.) 
         */
         *pcolor++ = ps->r;
         *pcolor++ = ps->g;
         *pcolor++ = ps->b;
         *pcolor++ = ps->a;
      }
   }
   if (NumOpen < max_colors)
   {  
      EL_printf ("Error: Not enough open slots (%d) in palette for %d quantized colors\n", NumOpen, max_colors);
      return 2;
   }
   
   if ((status = initialize(max_colors, palsite, num_pal_entries)) != 0)
      goto reduce_error;

   /*
   ** Seed preset colors if any 
   */
   if (NumSeeded)
   {
      int i;
      box *pbox;
      SEEDINFO *pseedinfo;
      
      pfnShrinkBox = no_shrink_box;
      pfnSumBox = sum_box_seeded;
      

      /*
      ** Split the RGB cube into boxes with one seed in each box.  The entire 
      ** cube must be represented by the union of the boxes, so the boxes do 
      ** not get shrunk in this pass of the quantization process.  
      */
      while (num_boxes < NumSeeded)
      {
         if ((pbox = worst_box()) == NULL)
            break;
   
         split_box(pbox);
      }
      ASSERT (num_boxes == NumSeeded);
      
      for (i = NumSeeded, pbox = boxes; i; i--, pbox++)
      {
         pbox->fHasSeed = TRUE;
      }
      
      // Restore histogram values at seed positions but make sure they are non zero.
      for (i = NumSeeded, pseedinfo = arseedinfo+1; i; i--, pseedinfo++)
      {
         *pseedinfo->phist = (0 == pseedinfo->histValue) ? 1 : pseedinfo->histValue;
      }
      sum_box (boxes); // always call original sum_box here. Don't go through func pointer.
   }

   pfnShrinkBox = shrink_box;
   pfnSumBox = sum_box;
   
   max_colors += NumSeeded; 
   while (num_boxes < max_colors)
   {
    if ((pbox = worst_box()) == NULL)
      break;
   
    split_box(pbox);
   }
   
   make_color_map(color_map, palsite, num_pal_entries);
   
   *num_colors = num_boxes;

reduce_error:

   if (arseedinfo) free(arseedinfo);
   if (boxes) free(boxes);
   if (heap) free(heap);
   
   return (int)status;
}
/*----------------------------------------------------------------------------*/

