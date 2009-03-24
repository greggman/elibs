/*************************************************************************
 *                                                                       *
 *                             Gfinfo.CPP                                *
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
      Reads in any GFX readable file format and outputs info about it in a
      simple to parse name=value format.

   PROGRAMMERS
      Gregg A. Tavares

   FUNCTIONS

   TABS : 4 7

   HISTORY
    08/01/96 : JMA Created.

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include <echidna/ensure.h>

#include <echidna/argparse.h>
#include <echidna/readgfx.h>
#include <echidna/eerrors.h>
#include <echidna/eio.h>
#include <echidna/checkglu.h>

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

#define ARG_INFILE    (newargs[ 0])

char Usage[] = "Usage: gfinfo INFILE\n";

ArgSpec Template[] = {
   {STANDARD_ARG|REQUIRED_ARG,        "INFILE",
      "\tINFILE  =  File to read\n", },
   {0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
BEGINFUNCMAIN(main)
{
  char      **newargs;

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
      BlockO32BitPixels *b32;

      b32 = Read32BitPicture (ARG(InFile));
      if (b32)
      {
        // Check if there is any non 1 alpha.
        bool nonOneAlpha = false;
        pixel32* pixel = b32->rgba;
        pixel32* lastPixel = pixel + b32->width * b32->height;

        while (pixel < lastPixel)
        {
          if (pixel->alpha != 255)
          {
            nonOneAlpha = true;
            break;
          }
          ++pixel;
        }

        EL_printf("width=%d\n", b32->width);
        EL_printf("height=%d\n", b32->height);
        EL_printf("nonOneAlpha=%d\n", nonOneAlpha);
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

