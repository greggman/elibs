/*************************************************************************
 *                                                                       *
 *                             GfxInOut.CPP                              *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION
      Reads in any GFX readable file format and outputs any GFX writeable
      file format.  The formats are  indicated by the extensions of the input
      and output file names supplied on the command line.
      
   PROGRAMMERS
      Gregg A. Tavares
      Juan M. Alvarado

   FUNCTIONS

   TABS : 4 7

   HISTORY
		08/01/96 : JMA Created.

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include <echidna\ensure.h>

#include <echidna\argparse.h>
#include <echidna\readgfx.h>
#include <echidna\eerrors.h>
#include <echidna\eio.h>
#include <echidna\checkglu.h>

/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/


/************************** P R O T O T Y P E S **************************/


/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/


/**************************** R O U T I N E S ****************************/


/*************************** ArgParse Template ***************************/
enum {
   NDX_InFile,
   NDX_OutFile,
   NDX_XStart,
   NDX_YStart,
   NDX_Width,
   NDX_Height,
};
#define ARG(name) (newargs [NDX_ ## name])
#define qprintf(arg_list)  (!ARG(Quiet)) ? EL_printf arg_list : NULL

#define ARG_INFILE		(newargs[ 0])
#define ARG_OUTFILE		(newargs[ 1])

char Usage[] = "Usage: LowSize INFILE OUTFILE\n";

ArgSpec Template[] = {
   {STANDARD_ARG|REQUIRED_ARG,				"INFILE",	
      "\tINFILE  = Binary File to read\n", },
   {STANDARD_ARG|REQUIRED_ARG,				"OUTFILE",	
      "\tOUTFILE = Binary File to write\n", },
   {CHRKEYWORD_ARG,              "X",	   
      "\t-X<x coord>  X start coordinate of sub image in infile to translate (Default = 0).\n"
   ,},
   {CHRKEYWORD_ARG,              "Y",	   
      "\t-Y<y coord>  Y start coordinate of sub image in infile to translate (Default = 0).\n"
   ,},
   {CHRKEYWORD_ARG,              "W",	   
      "\t-W<width>    Width of sub image in infile to translate (Default = infile image width - X start coordinate).\n"
   ,},
   {CHRKEYWORD_ARG,              "H",	   
      "\t-H<height>   Height of sub image in infile to translate (Default = infile image height - Y start coordinate).\n"
   ,},
   
   {0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
BEGINFUNCMAIN(main)
{
	char			**newargs;

	newargs = argparse (argc, argv, Template);

	if (!newargs)
	{
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		return EXIT_FAILURE;
	}
	else
	{
		// Load a 32 bit picture (or 8bit as 32bit) and then save it
		{
			BlockO32BitPixels	*b32;
	
			b32 = Read32BitPicture (ARG(InFile));
			if (b32)
			{
            int x, y, width, height;
            
            /* Get image cropping parameters */
            x = ARG(XStart) ? atoi (ARG(XStart)) : 0;
            y = ARG(YStart) ? atoi (ARG(YStart)) : 0;
            width = ARG(Width) ? atoi (ARG(Width)) : (b32->width - x);
            height = ARG(Height) ? atoi (ARG(Height)) : (b32->height - y);
            
            /* Check validity of crop parameters */
            if (x < 0 || x >= b32->width)
            {
               EL_printf ("ERROR: -X %d is out of range (0..%ld) for image %s.\n", x, b32->width-1, ARG(InFile));
               return EXIT_FAILURE;
            }
            if (y < 0 || y >= b32->height)
            {
               EL_printf ("ERROR: -Y %d is out of range (0..%ld) for image %s.\n", y, b32->height-1, ARG(InFile));
               return EXIT_FAILURE;
            }
            if (width < 1 || width > (b32->width - x))
            {
               EL_printf ("ERROR: -W %d is out of range (1..%ld) for image %s.\n", width, b32->width-x, ARG(InFile));
               return EXIT_FAILURE;
            }
            if (height < 1 || height > (b32->height - y))
            {
               EL_printf ("ERROR: -H %d is out of range (1..%ld) for image %s.\n", height, b32->height-y, ARG(InFile));
               return EXIT_FAILURE;
            }
            
            /* Check if need to crop the image */
            if (x > 0 || y > 0 || width != b32->width || height != b32->height)
            { /* Crop the image */
               pixel32 *p32Dst;
               pixel32 *p32Src;
               int i, j;
               int dxMargin;
               
               
               /* Copy sub-image to top of buffer of original image */
               for (
                  p32Src = b32->rgba + y * b32->width + x,
                     p32Dst = b32->rgba, 
                     i = height,
                     dxMargin = b32->width - width;
                  i;
                  i--, 
                     p32Src += dxMargin
               )
               {
                  for (j = width; j; j--)
                  {
                     *p32Dst++ = *p32Src++;
                  }
               }
               b32->width = width;
               b32->height = height;
            }
               
				Write32BitPicture (ARG(OutFile), b32);
			}
        else
        {
            EL_printf("ERROR: unable to load picture %s\n", ARG(InFile)); 
        }
		}
	}

	if (GlobalErr) {
		EL_printf ("%s\n", GlobalErrMsg);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
ENDFUNCMAIN(main)

