/**************************************************************************
 *                                                                        *
 *                               PLATFORM.H                               *
 *                                                                        *
 **************************************************************************

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

      Target platform specification defines and base type defines and
      macros that make possible platform independent code.

   PROGRAMMERS

      Juan M. Alvarado, Gregg Tavares

   FUNCTIONS


   HISTORY

		07/09/96 GAT: Created

 **************************************************************************/


#ifndef EL_PLATFORM_H
#define EL_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** I N C L U D E S ****************************/


/**************************** C O N S T A N T S ***************************/

/*
 * Platform: Set this in your MAKEFILE or in an environment var that
 * is used in your makefile.  That way this file doesn't have to change
 * on each machine
 */

   #if defined(_EL_PLAT_SONY__)

		/* Processor:
		 *    Set one of the defines below to 1, the rest to 0.
		 */

		   #define _EL_CPU_iAPx86__   0
		   #define _EL_CPU_M68000__   0
		   #define _EL_CPU_ARM60__    0
		   #define _EL_CPU_PPC602__   0
		   #define _EL_CPU_r4400__    0
		   #define _EL_CPU_R3000__    1

		/* Operating System:
		 *    Set one of the defines below to 1, the rest to 0.
		 */


		   #define _EL_OS_MSDOS__          0
		   #define _EL_OS_WIN32__          0
		   #define _EL_OS_AMIGAOS__        0
		   #define _EL_OS_MACOS__          0
		   #define _EL_OS_IRIX53__         0
		   #define _EL_OS_PSXOS__          1

		/* Compiler:
		 *    Set one of the defines below to 1, the rest to 0.
		 */

		   #define _EL_CC_TURBOC__      0     // Borland
		   #define _EL_CC_ARMC__        0     // Arm
		   #define _EL_CC_WATCOMC__     0     // Watcom
		   #define _EL_CC_ZTC__         0     // Zortech
		   #define _EL_CC_MACSC__       0     // Macintosh
		   #define _EL_CC_DIABC__       0     // Diab
		   #define _EL_CC_SGIC__        0     // SGI C
		   #define _EL_CC_VC__          0     // Microsoft VC++
		   #define _EL_CC_CCPSX__       1     // Psygnosis Psy-Q

		/* Include Syntax:
		 *    Set one of the defines below to 1, the rest to 0.
		 */
		   #define _EL_INCSYNTAX_A__    0     // e.g. echidna/platform.h
		   #define _EL_INCSYNTAX_B__    0     // e.g. echidna:platform.h
		   #define _EL_INCSYNTAX_C__    1     // e.g. echidna\platform.h
		   #define _EL_INCSYNTAX_D__    0     // e.g. :echidna:platform.h

	#elif defined(_EL_PLAT_M2__)

		/* Processor:
		 *    Set one of the defines below to 1, the rest to 0.
		 */

		   #define _EL_CPU_iAPx86__   0
		   #define _EL_CPU_M68000__   0
		   #define _EL_CPU_ARM60__    0
		   #define _EL_CPU_PPC602__   1
		   #define _EL_CPU_r4400__    0
		   #define _EL_CPU_R3000__    0

		/* Operating System:
		 *    Set one of the defines below to 1, the rest to 0.
		 */


		   #define _EL_OS_MSDOS__          0
		   #define _EL_OS_WIN32__          0
		   #define _EL_OS_AMIGAOS__        0
		   #define _EL_OS_MACOS__          0
		   #define _EL_OS_IRIX53__         0
		   #define _EL_OS_PSXOS__          0

		/* Compiler:
		 *    Set one of the defines below to 1, the rest to 0.
		 */

		   #define _EL_CC_TURBOC__      0     // Borland
		   #define _EL_CC_ARMC__        0     // Arm
		   #define _EL_CC_WATCOMC__     0     // Watcom
		   #define _EL_CC_ZTC__         0     // Zortech
		   #define _EL_CC_MACSC__       0     // Macintosh
		   #define _EL_CC_DIABC__       1     // Diab
		   #define _EL_CC_SGIC__        0     // SGI C
		   #define _EL_CC_VC__          0     // Microsoft VC++
		   #define _EL_CC_CCPSX__       0     // Psygnosis Psy-Q

		/* Include Syntax:
		 *    Set one of the defines below to 1, the rest to 0.
		 */
		   #define _EL_INCSYNTAX_A__    0     // e.g. echidna/platform.h
		   #define _EL_INCSYNTAX_B__    1     // e.g. echidna:platform.h
		   #define _EL_INCSYNTAX_C__    0     // e.g. echidna\platform.h
		   #define _EL_INCSYNTAX_D__    0     // e.g. :echidna:platform.h

   #elif defined(_EL_PLAT_SGI__)

		/* Processor:
		 *    Set one of the defines below to 1, the rest to 0.
		 */

		   #define _EL_CPU_iAPx86__   0
		   #define _EL_CPU_M68000__   0
		   #define _EL_CPU_ARM60__    0
		   #define _EL_CPU_PPC602__   0
		   #define _EL_CPU_r4400__    1
		   #define _EL_CPU_R3000__    0

		/* Operating System:
		 *    Set one of the defines below to 1, the rest to 0.
		 */


		   #define _EL_OS_MSDOS__          0
		   #define _EL_OS_WIN32__          0
		   #define _EL_OS_AMIGAOS__        0
		   #define _EL_OS_MACOS__          0
		   #define _EL_OS_IRIX53__         1
		   #define _EL_OS_PSXOS__          0

		/* Compiler:
		 *    Set one of the defines below to 1, the rest to 0.
		 */

		   #define _EL_CC_TURBOC__      0     // Borland
		   #define _EL_CC_ARMC__        0     // Arm
		   #define _EL_CC_WATCOMC__     0     // Watcom
		   #define _EL_CC_ZTC__         0     // Zortech
		   #define _EL_CC_MACSC__       0     // Macintosh
		   #define _EL_CC_DIABC__       0     // Diab
		   #define _EL_CC_SGIC__        1     // SGI C
		   #define _EL_CC_VC__          0     // Microsoft VC++
		   #define _EL_CC_CCPSX__       0     // Psygnosis Psy-Q

		/* Include Syntax:
		 *    Set one of the defines below to 1, the rest to 0.
		 */
		   #define _EL_INCSYNTAX_A__    1     // e.g. echidna/platform.h
		   #define _EL_INCSYNTAX_B__    0     // e.g. echidna:platform.h
		   #define _EL_INCSYNTAX_C__    0     // e.g. echidna\platform.h
		   #define _EL_INCSYNTAX_D__    0     // e.g. :echidna:platform.h

	#elif defined(_EL_PLAT_M2__)

		/* Processor:
		 *    Set one of the defines below to 1, the rest to 0.
		 */

		   #define _EL_CPU_iAPx86__   0
		   #define _EL_CPU_M68000__   0
		   #define _EL_CPU_ARM60__    0
		   #define _EL_CPU_PPC602__   1
		   #define _EL_CPU_r4400__    0
		   #define _EL_CPU_R3000__    0

		/* Operating System:
		 *    Set one of the defines below to 1, the rest to 0.
		 */


		   #define _EL_OS_MSDOS__          0
		   #define _EL_OS_WIN32__          0
		   #define _EL_OS_AMIGAOS__        0
		   #define _EL_OS_MACOS__          0
		   #define _EL_OS_IRIX53__         0
		   #define _EL_OS_PSXOS__          0

		/* Compiler:
		 *    Set one of the defines below to 1, the rest to 0.
		 */

		   #define _EL_CC_TURBOC__      0     // Borland
		   #define _EL_CC_ARMC__        0     // Arm
		   #define _EL_CC_WATCOMC__     0     // Watcom
		   #define _EL_CC_ZTC__         0     // Zortech
		   #define _EL_CC_MACSC__       0     // Macintosh
		   #define _EL_CC_DIABC__       1     // Diab
		   #define _EL_CC_SGIC__        0     // SGI C
		   #define _EL_CC_VC__          0     // Microsoft VC++
		   #define _EL_CC_CCPSX__       0     // Psygnosis Psy-Q

		/* Include Syntax:
		 *    Set one of the defines below to 1, the rest to 0.
		 */
		   #define _EL_INCSYNTAX_A__    0     // e.g. echidna/platform.h
		   #define _EL_INCSYNTAX_B__    1     // e.g. echidna:platform.h
		   #define _EL_INCSYNTAX_C__    0     // e.g. echidna\platform.h
		   #define _EL_INCSYNTAX_D__    0     // e.g. :echidna:platform.h

	#elif defined(_EL_PLAT_MAC68__)	// Mac with 68000

		/* Processor:
		 *    Set one of the defines below to 1, the rest to 0.
		 */

		   #define _EL_CPU_iAPx86__   0
		   #define _EL_CPU_M68000__   1
		   #define _EL_CPU_ARM60__    0
		   #define _EL_CPU_PPC602__   0
		   #define _EL_CPU_r4400__    0
		   #define _EL_CPU_R3000__    0

		/* Operating System:
		 *    Set one of the defines below to 1, the rest to 0.
		 */


		   #define _EL_OS_MSDOS__          0
		   #define _EL_OS_WIN32__          0
		   #define _EL_OS_AMIGAOS__        0
		   #define _EL_OS_MACOS__          1
		   #define _EL_OS_IRIX53__         0
		   #define _EL_OS_PSXOS__          0

		/* Compiler:
		 *    Set one of the defines below to 1, the rest to 0.
		 */

		   #define _EL_CC_TURBOC__      0     // Borland
		   #define _EL_CC_ARMC__        0     // Arm
		   #define _EL_CC_WATCOMC__     0     // Watcom
		   #define _EL_CC_ZTC__         0     // Zortech
		   #define _EL_CC_MACSC__       1     // Macintosh
		   #define _EL_CC_DIABC__       0     // Diab
		   #define _EL_CC_SGIC__        0     // SGI C
		   #define _EL_CC_VC__          0     // Microsoft VC++
		   #define _EL_CC_CCPSX__       0     // Psygnosis Psy-Q

		/* Include Syntax:
		 *    Set one of the defines below to 1, the rest to 0.
		 */
		   #define _EL_INCSYNTAX_A__    0     // e.g. echidna/platform.h
		   #define _EL_INCSYNTAX_B__    0     // e.g. echidna:platform.h
		   #define _EL_INCSYNTAX_C__    0     // e.g. echidna\platform.h
		   #define _EL_INCSYNTAX_D__    1     // e.g. :echidna:platform.h

	#elif defined(_EL_PLAT_PCB16__)	/* PC with Borland */

		/* Processor:
		 *    Set one of the defines below to 1, the rest to 0.
		 */

		   #define _EL_CPU_iAPx86__   1
		   #define _EL_CPU_M68000__   0
		   #define _EL_CPU_ARM60__    0
		   #define _EL_CPU_PPC602__   0
		   #define _EL_CPU_r4400__    0
		   #define _EL_CPU_R3000__    0

		/* Operating System:
		 *    Set one of the defines below to 1, the rest to 0.
		 */


		   #define _EL_OS_MSDOS__          1
		   #define _EL_OS_WIN32__          0
		   #define _EL_OS_AMIGAOS__        0
		   #define _EL_OS_MACOS__          0
		   #define _EL_OS_IRIX53__         0
		   #define _EL_OS_PSXOS__          0

		/* Compiler:
		 *    Set one of the defines below to 1, the rest to 0.
		 */

		   #define _EL_CC_TURBOC__      1     // Borland
		   #define _EL_CC_ARMC__        0     // Arm
		   #define _EL_CC_WATCOMC__     0     // Watcom
		   #define _EL_CC_ZTC__         0     // Zortech
		   #define _EL_CC_MACSC__       0     // Macintosh
		   #define _EL_CC_DIABC__       0     // Diab
		   #define _EL_CC_SGIC__        0     // SGI C
		   #define _EL_CC_VC__          0     // Microsoft VC++
		   #define _EL_CC_CCPSX__       0     // Psygnosis Psy-Q

		/* Include Syntax:
		 *    Set one of the defines below to 1, the rest to 0.
		 */
		   #define _EL_INCSYNTAX_A__    0     // e.g. echidna/platform.h
		   #define _EL_INCSYNTAX_B__    0     // e.g. echidna:platform.h
		   #define _EL_INCSYNTAX_C__    1     // e.g. echidna\platform.h
		   #define _EL_INCSYNTAX_D__    0     // e.g. :echidna:platform.h

	#elif defined(_EL_PLAT_PCB32__)	/* PC with Borland 32 bits */

		/* Processor:
		 *    Set one of the defines below to 1, the rest to 0.
		 */

		   #define _EL_CPU_iAPx86__   1
		   #define _EL_CPU_M68000__   0
		   #define _EL_CPU_ARM60__    0
		   #define _EL_CPU_PPC602__   0
		   #define _EL_CPU_r4400__    0
		   #define _EL_CPU_R3000__    0

		/* Operating System:
		 *    Set one of the defines below to 1, the rest to 0.
		 */


		   #define _EL_OS_MSDOS__          1
		   #define _EL_OS_WIN32__          0
		   #define _EL_OS_AMIGAOS__        0
		   #define _EL_OS_MACOS__          0
		   #define _EL_OS_IRIX53__         0
		   #define _EL_OS_PSXOS__          0

		/* Compiler:
		 *    Set one of the defines below to 1, the rest to 0.
		 */

		   #define _EL_CC_TURBOC__      1     // Borland
		   #define _EL_CC_ARMC__        0     // Arm
		   #define _EL_CC_WATCOMC__     0     // Watcom
		   #define _EL_CC_ZTC__         0     // Zortech
		   #define _EL_CC_MACSC__       0     // Macintosh
		   #define _EL_CC_DIABC__       0     // Diab
		   #define _EL_CC_SGIC__        0     // SGI C
		   #define _EL_CC_VC__          0     // Microsoft VC++
		   #define _EL_CC_CCPSX__       0     // Psygnosis Psy-Q

		/* Include Syntax:
		 *    Set one of the defines below to 1, the rest to 0.
		 */
		   #define _EL_INCSYNTAX_A__    0     // e.g. echidna/platform.h
		   #define _EL_INCSYNTAX_B__    0     // e.g. echidna:platform.h
		   #define _EL_INCSYNTAX_C__    1     // e.g. echidna\platform.h
		   #define _EL_INCSYNTAX_D__    0     // e.g. :echidna:platform.h

	#elif defined(_EL_PLAT_PCM32__)	/* PC with Microsoft 32 bits */

		/* Processor:
		 *    Set one of the defines below to 1, the rest to 0.
		 */

		   #define _EL_CPU_iAPx86__   1
		   #define _EL_CPU_M68000__   0
		   #define _EL_CPU_ARM60__    0
		   #define _EL_CPU_PPC602__   0
		   #define _EL_CPU_r4400__    0
		   #define _EL_CPU_R3000__    0

		/* Operating System:
		 *    Set one of the defines below to 1, the rest to 0.
		 */


		   #define _EL_OS_MSDOS__          1
		   #define _EL_OS_WIN32__          0
		   #define _EL_OS_AMIGAOS__        0
		   #define _EL_OS_MACOS__          0
		   #define _EL_OS_IRIX53__         0
		   #define _EL_OS_PSXOS__          0

		/* Compiler:
		 *    Set one of the defines below to 1, the rest to 0.
		 */

		   #define _EL_CC_TURBOC__      0     // Borland
		   #define _EL_CC_ARMC__        0     // Arm
		   #define _EL_CC_WATCOMC__     0     // Watcom
		   #define _EL_CC_ZTC__         0     // Zortech
		   #define _EL_CC_MACSC__       0     // Macintosh
		   #define _EL_CC_DIABC__       0     // Diab
		   #define _EL_CC_SGIC__        0     // SGI C
		   #define _EL_CC_VC__          0     // Microsoft VC++
		   #define _EL_CC_CCPSX__       0     // Psygnosis Psy-Q

		/* Include Syntax:
		 *    Set one of the defines below to 1, the rest to 0.
		 */
		   #define _EL_INCSYNTAX_A__    0     // e.g. echidna/platform.h
		   #define _EL_INCSYNTAX_B__    0     // e.g. echidna:platform.h
		   #define _EL_INCSYNTAX_C__    1     // e.g. echidna\platform.h
		   #define _EL_INCSYNTAX_D__    0     // e.g. :echidna:platform.h

	#elif defined(_EL_PLAT_WIN32__)	/* PC with Microsoft 32 bits */

		/* Processor:
		 *    Set one of the defines below to 1, the rest to 0.
		 */

		   #define _EL_CPU_iAPx86__   1
		   #define _EL_CPU_M68000__   0
		   #define _EL_CPU_ARM60__    0
		   #define _EL_CPU_PPC602__   0
		   #define _EL_CPU_r4400__    0
		   #define _EL_CPU_R3000__    0

		/* Operating System:
		 *    Set one of the defines below to 1, the rest to 0.
		 */


		   #define _EL_OS_MSDOS__          0
		   #define _EL_OS_WIN32__          1
		   #define _EL_OS_AMIGAOS__        0
		   #define _EL_OS_MACOS__          0
		   #define _EL_OS_IRIX53__         0
		   #define _EL_OS_PSXOS__          0

		/* Compiler:
		 *    Set one of the defines below to 1, the rest to 0.
		 */

		   #define _EL_CC_TURBOC__      0     // Borland
		   #define _EL_CC_ARMC__        0     // Arm
		   #define _EL_CC_WATCOMC__     0     // Watcom
		   #define _EL_CC_ZTC__         0     // Zortech
		   #define _EL_CC_MACSC__       0     // Macintosh
		   #define _EL_CC_DIABC__       0     // Diab
		   #define _EL_CC_SGIC__        0     // SGI C
		   #define _EL_CC_VC__          1     // Microsoft VC++
		   #define _EL_CC_CCPSX__       0     // Psygnosis Psy-Q

		/* Include Syntax:
		 *    Set one of the defines below to 1, the rest to 0.
		 */
		   #define _EL_INCSYNTAX_A__    0     // e.g. echidna/platform.h
		   #define _EL_INCSYNTAX_B__    0     // e.g. echidna:platform.h
		   #define _EL_INCSYNTAX_C__    1     // e.g. echidna\platform.h
		   #define _EL_INCSYNTAX_D__    0     // e.g. :echidna:platform.h

			#if __cplusplus
				}

				#ifdef _AFXDLL
					#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

					#include <afxwin.h>         // MFC core and standard components
					#include <afxext.h>         // MFC extensions
					#include <afxdisp.h>        // MFC OLE automation classes
					#include <afxadv.h>
					#include <afxdlgs.h>        // CommDlg Wrappers
					#include <afxcoll.h>
					#include <afxtempl.h>
					#ifndef _AFX_NO_AFXCMN_SUPPORT
						#include <afxcmn.h>			// MFC support for Windows 95 Common Controls
					#endif // _AFX_NO_AFXCMN_SUPPORT

					#define _EL_INCLUDED_LIBS = 1
					#if 0 // this needs to be fixed
					#if defined(_DEBUG)

						#pragma (lib, "mfcelibd")

					#else

						#pragma (lib, "mfcelib")

					#endif
					#endif

				#else
					#include <windows.h>

				#endif

				extern "C" {
			#else
				#include <windows.h>
			#endif

			#ifndef _EL_INCLUDED_LIBS
				#if    defined(_DEBUG) &&  defined(_MT) &&  defined (_DLL)
					#pragma comment(lib, "elibmtdlld")
				#elif  defined(_DEBUG) &&  defined(_MT) && !defined (_DLL)
					#pragma comment(lib, "elibmtd")
				#elif  defined(_DEBUG) && !defined(_MT) &&  defined (_DLL)
					#pragma comment(lib, "elibd")	// no non MT dll
				#elif  defined(_DEBUG) && !defined(_MT) && !defined (_DLL)
					#pragma comment(lib, "elibd")
				#elif !defined(_DEBUG) &&  defined(_MT) &&  defined (_DLL)
					#pragma comment(lib, "elibmtdll")
				#elif !defined(_DEBUG) &&  defined(_MT) && !defined (_DLL)
					#pragma comment(lib, "elibmt")
				#elif !defined(_DEBUG) && !defined(_MT) &&  defined (_DLL)
					#pragma comment(lib, "elib")	// no non MT dll
				#elif !defined(_DEBUG) && !defined(_MT) && !defined (_DLL)
					#pragma comment(lib, "elib")
				#else
					#error should not be here
				#endif
			#endif

	#elif defined(_EL_PLAT_PCW32__)	/* PC with Watcom 32 bits */

		/* Processor:
		 *    Set one of the defines below to 1, the rest to 0.
		 */

		   #define _EL_CPU_iAPx86__   1
		   #define _EL_CPU_M68000__   0
		   #define _EL_CPU_ARM60__    0
		   #define _EL_CPU_PPC602__   0
		   #define _EL_CPU_r4400__    0
		   #define _EL_CPU_R3000__    0

		/* Operating System:
		 *    Set one of the defines below to 1, the rest to 0.
		 */


		   #define _EL_OS_MSDOS__          1
		   #define _EL_OS_WIN32__          0
		   #define _EL_OS_AMIGAOS__        0
		   #define _EL_OS_MACOS__          0
		   #define _EL_OS_IRIX53__         0
		   #define _EL_OS_PSXOS__          0

		/* Compiler:
		 *    Set one of the defines below to 1, the rest to 0.
		 */

		   #define _EL_CC_TURBOC__      0     // Borland
		   #define _EL_CC_ARMC__        0     // Arm
		   #define _EL_CC_WATCOMC__     1     // Watcom
		   #define _EL_CC_ZTC__         0     // Zortech
		   #define _EL_CC_MACSC__       0     // Macintosh
		   #define _EL_CC_DIABC__       0     // Diab
		   #define _EL_CC_SGIC__        0     // SGI C
		   #define _EL_CC_VC__          0     // Microsoft VC++
		   #define _EL_CC_CCPSX__       0     // Psygnosis Psy-Q

		/* Include Syntax:
		 *    Set one of the defines below to 1, the rest to 0.
		 */
		   #define _EL_INCSYNTAX_A__    0     // e.g. echidna/platform.h
		   #define _EL_INCSYNTAX_B__    0     // e.g. echidna:platform.h
		   #define _EL_INCSYNTAX_C__    1     // e.g. echidna\platform.h
		   #define _EL_INCSYNTAX_D__    0     // e.g. :echidna:platform.h

   #else
 	   #error It would be best if you choose a platform.
	#endif


/* Target Specific Base Type Interface:
 *
 *    FAR32 and HUGE32 should be defined so that the expression
 *
 *       char FAR32 *sz;
 *
 *    returns a 32-bit char pointer, and
 *    MK_FP32(unsigned seg, unsigned off) should return a FAR32 pointer.
 *
 *    Thus on 16-bit Borland C++, FAR32 == "far" and HUGE32 == "huge",
 *    while on Watcom C/C++32, FAR32 == "" and HUGE32 == ""
 *    (because far on Watcom C/C++32 is actually 48-bits!)
 */
/*
 * Note: __LSBFIRST__ is 1 if the native byte order of
 *    the target platform is least significant byte first.
 *
 */
   #if _EL_CPU_ARM60__

      #define FAR32
      #define HUGE32
      #define MK_FP32(seg,off) ((void *) (((seg)<<4)|(off)))

      #define  __LSBFIRST__   0
      #define __WORD_ALIGN_SIZE__      4
      #define __mWordPtrUnAligned__    0x03UL
      #define __mWordPtrAlign__        0xFFFFFFFCUL

      typedef signed    char     INT8;
      typedef unsigned  char     UINT8;
      typedef signed    short    INT16;   // avoid using on ARM
      typedef unsigned  short    UINT16;   // avoid using on ARM
      typedef           long     INT32;
      typedef unsigned  long     UINT32;
      typedef           int      BOOL;

	#elif _EL_CPU_PPC602__
      #define FAR32
      #define HUGE32
      #define MK_FP32(seg,off) ((void *) ((off)))

      #define  __LSBFIRST__   0
      #define __WORD_ALIGN_SIZE__      4
      #define __mWordPtrUnAligned__    0x03UL
      #define __mWordPtrAlign__        0xFFFFFFFCUL

      typedef signed    char     INT8;
      typedef unsigned  char     UINT8;
      typedef signed    short    INT16;
      typedef unsigned  short    UINT16;
      typedef signed    long     INT32;
      typedef unsigned  long     UINT32;
      typedef           int      BOOL;

      typedef signed    char     INT8;
      typedef unsigned  char     UINT8;
      typedef signed    short    INT16;
      typedef unsigned  short    UINT16;
      typedef signed    long     INT32;
      typedef unsigned  long     UINT32;
      typedef           int      BOOL;


   #elif _EL_CPU_M68000__

      #define __WORD_ALIGN_SIZE__ 2
      #if _EL_CC_TURBOC__

      #elif _EL_CC_WATCOMC__

      #elif _EL_CC_ZTC__

      #else
         #error Platform Compiler not specified.
      #endif

   #elif _EL_CPU_iAPx86__

      #define __LSBFIRST__    1
      #define __WORD_ALIGN_SIZE__   1
      #define __mWordPtrUnAligned__    0x00UL
      #define __mWordPtrAlign__        0xFFFFFFFFUL

      #if _EL_CC_TURBOC__

      #elif _EL_CC_WATCOMC__

		 #define far
		 #define huge
         #define FAR32
         #define HUGE32
         #define MK_FP32(seg,off) ((void *) (((seg)<<4)+(off)))

         typedef signed    char     INT8;
         typedef unsigned  char     UINT8;
         typedef signed    short    INT16;
         typedef unsigned  short    UINT16;
         typedef           long     INT32;
         typedef unsigned  long     UINT32;
         typedef           UINT8    BOOL;

      #elif _EL_CC_ZTC__

	  #elif _EL_CC_VC__

  		 #define TRUE  1
   		 #define FALSE 0

		 #define far
		 #define huge
         #define FAR32
         #define HUGE32
//         #define MK_FP32(seg,off) ((void *) (((seg)<<4)+(off)))

         typedef signed    char     INT8;
         typedef unsigned  char     UINT8;
         typedef signed    short    INT16;
         typedef unsigned  short    UINT16;
         //typedef           long     INT32;
         //typedef unsigned  long     UINT32;

      #else
         #error Platform Compiler not specified.
      #endif

      #define int8   INT8
      #define uint8  UINT8
      #define int16  INT16
      #define uint16 UINT16
      #define int32  INT32
      #define uint32 UINT32
      #ifndef __cplusplus
          #define bool   BOOL
      #endif

   #elif _EL_CPU_r4400__

	   #define far
	   #define huge

      #define FAR32
      #define HUGE32
      #define MK_FP32(seg,off) ((void *) ((off)))

      #define  __LSBFIRST__   0
      #define __WORD_ALIGN_SIZE__      4
      #define __mWordPtrUnAligned__    0x03UL
      #define __mWordPtrAlign__        0xFFFFFFFCUL

      typedef signed    char     INT8;
      typedef unsigned  char     UINT8;
      typedef signed    short    INT16;
      typedef unsigned  short    UINT16;
      typedef signed    long     INT32;
      typedef unsigned  long     UINT32;
      typedef           int      BOOL;

      #define int8   INT8
      #define uint8  UINT8
      #define int16  INT16
      #define uint16 UINT16
      #define int32  INT32
      #define uint32 UINT32

      #ifndef __cplusplus
          #define bool   BOOL
      #endif

	   #define TRUE  1
	   #define FALSE 0

   #elif _EL_CPU_R3000__

      #define __LSBFIRST__    1
      #define __WORD_ALIGN_SIZE__   4
      #define __mWordPtrUnAligned__    0x03UL
      #define __mWordPtrAlign__        0xFFFFFFFCUL

      #if _EL_CC_TURBOC__

      #elif _EL_CC_WATCOMC__

      #elif _EL_CC_ZTC__

	   #elif _EL_CC_VC__

		#elif _EL_CC_CCPSX__

		   #define TRUE  1
		   #define FALSE 0

		   #define near
		   #define far
		   #define huge
		   #define FAR32
		   #define HUGE32
//	      #define MK_FP32(seg,off) ((void *) (((seg)<<4)+(off)))

			typedef unsigned	char		UINT;
			typedef unsigned	short		WORD;
			typedef signed    char     INT8;
			typedef unsigned  char     UINT8;
			typedef signed    short    INT16;
			typedef unsigned  short    UINT16;
			typedef signed    long     INT32;
			typedef unsigned  long     UINT32;
			typedef           int      BOOL;
			typedef unsigned	long		DWORD;

			#include <stddef.h>

      #else
         #error Platform Compiler not specified.
      #endif

      #define int8   INT8
      #define uint8  UINT8
      #define int16  INT16
      #define uint16 UINT16
      #define int32  INT32
      #define uint32 UINT32
      #ifndef __cplusplus
          #define bool   BOOL
      #endif

   #else
      #error Platform CPU not specified.
   #endif

#define INT8MAX      0x7F
#define INT8MIN      0x80
#define INT16MIN     0x8000
#define INT16MAX     0x7FFF
#define INT32MIN     0x80000000L
#define INT32MAX     0x7FFFFFFFL
#define INT32MIN     0x80000000L
#define INT64MAX     0x7FFFFFFFFFFFFFFFLL
#define INT64MIN     0x8000000000000000LL

#define UINT8MAX     0xFF
#define UINT16MAX    0xFFFF
#define UINT32MAX    0xFFFFFFFFUL
#define UINT64MAX    0xFFFFFFFFFFFFFFFFULL

#define IsUnAligned(arg)   ((arg) & __mWordPtrUnAligned__)
#define TruncAlign(arg)    ((arg) & __mWordPtrAlign__)
#define AlignedUp(arg)     ((IsUnAligned(arg)) ? (((UINT32)TruncAlign(arg)) + __WORD_ALIGN_SIZE__) : (arg))
#define AlignedDown(arg)   ((IsUnAligned(arg)) ? (TruncAlign(arg))   : (arg))

#define IsUnAlignedPtr(ptr)   IsUnAligned((UINT32)ptr)
#define TruncAlignPtr(ptr)    (void *)TruncAlign((UINT32)ptr)
#define AlignedUpPtr(ptr)     (void *)AlignedUp((UINT32)ptr)
#define AlignedDownPtr(ptr)   (void *)AlignedDown((UINT32)ptr)

#define IsUnAlignedVal(val)   IsUnAligned(val)
#define TruncAlignVal(val)    TruncAlign(val)
#define AlignedUpVal(val)     AlignedUp(val)
#define AlignedDownVal(val)   AlignedDown(val)

/************************************  ************************************/
/************************* Word/Long Sexify Macros ************************/
/************************************  ************************************/

/* Note:
 *    LSBF means Least significant Byte First.
 *    MSBF means Most significant Byte First.
 *    Native means natural byte order for platform.
 */
#if __LSBFIRST__

   #define MSBFToNative32Bit(a)    \
               ( ((((UINT32)(a)) & 0xFF000000L) >> 24L) |   \
               ((((UINT32)(a)) & 0x00FF0000L) >>  8L) |  \
               ((((UINT32)(a)) & 0x0000FF00L) <<  8L) |  \
               ((((UINT32)(a)) & 0x000000FFL) << 24L) )

   #define MSBFToNative16Bit(a)    \
               ( ((((UINT16)(a)) & 0x00FF) << 8) | \
               ((((UINT16)(a)) & 0xFF00) >> 8) )

   #define LSBFToNative32Bit(a) (a)
   #define LSBFToNative16Bit(a) (a)


   #define NativeToMSBF32Bit(a)    \
               ( ((((UINT32)(a)) & 0xFF000000L) >> 24L) |   \
               ((((UINT32)(a)) & 0x00FF0000L) >>  8L) |  \
               ((((UINT32)(a)) & 0x0000FF00L) <<  8L) |  \
               ((((UINT32)(a)) & 0x000000FFL) << 24L) )

   #define NativeToMSBF16Bit(a)    \
               ( ((((UINT16)(a)) & 0x00FF) << 8) | \
               ((((UINT16)(a)) & 0xFF00) >> 8) )

   #define NativeToLSBF32Bit(a) (a)
   #define NativeToLSBF16Bit(a) (a)


	 // old echidna stuff

	 #define __LITTLEENDIAN__ 1

	#define	BigLong(a)		(      ( ((((uint32)(a)) & 0xFF000000UL) >> 24) |	\
									 ((((uint32)(a)) & 0x00FF0000UL) >>  8) |	\
									 ((((uint32)(a)) & 0x0000FF00UL) <<  8) |	\
									 ((((uint32)(a)) & 0x000000FFUL) << 24) ))
	#define BigWord(a)		(      ( ((((uint16)(a)) & 0x00FF) << 8) |	\
									 ((((uint16)(a)) & 0xFF00) >> 8) ))

	#define LilLong(a)		(a)
	#define	LilWord(a)		(a)

	#define BigLong2Native(a)	((a) = ( ((((uint32)(a)) & 0xFF000000UL) >> 24) |	\
										 ((((uint32)(a)) & 0x00FF0000UL) >>  8) |	\
										 ((((uint32)(a)) & 0x0000FF00UL) <<  8) |	\
										 ((((uint32)(a)) & 0x000000FFUL) << 24) ))
	#define BigWord2Native(a)	((a) = ( ((((uint16)(a)) & 0x00FF) << 8) |	\
										 ((((uint16)(a)) & 0xFF00) >> 8) ))

	#define	LilLong2Native(a)	((a)=(a))
	#define	LilWord2Native(a)	((a)=(a))

#else

   #define LSBFToNative32Bit(a)    \
               ( ((((UINT32)(a)) & 0xFF000000L) >> 24L) |   \
               ((((UINT32)(a)) & 0x00FF0000L) >>  8L) |  \
               ((((UINT32)(a)) & 0x0000FF00L) <<  8L) |  \
               ((((UINT32)(a)) & 0x000000FFL) << 24L) )

   #define LSBFToNative16Bit(a)  \
               ( ((((UINT16)(a)) & 0x00FF) << 8) | \
               ((((UINT16)(a)) & 0xFF00) >> 8) )

   #define MSBFToNative32Bit(a) (a)
   #define MSBFToNative16Bit(a) (a)

   #define NativeToLSBF32Bit(a)    \
               ( ((((UINT32)(a)) & 0xFF000000L) >> 24L) |   \
               ((((UINT32)(a)) & 0x00FF0000L) >>  8L) |  \
               ((((UINT32)(a)) & 0x0000FF00L) <<  8L) |  \
               ((((UINT32)(a)) & 0x000000FFL) << 24L) )

   #define NativeToLSBF16Bit(a)  \
               ( ((((UINT16)(a)) & 0x00FF) << 8) | \
               ((((UINT16)(a)) & 0xFF00) >> 8) )

   #define NativeToMSBF32Bit(a) (a)
   #define NativeToMSBF16Bit(a) (a)

	 // old echidna stuff

	 #define __LITTLEENDIAN__ 0

	#define	LilLong(a)		(      ( ((((uint32)(a)) & 0xFF000000L) >> 24) |	\
									 ((((uint32)(a)) & 0x00FF0000L) >>  8) |	\
									 ((((uint32)(a)) & 0x0000FF00L) <<  8) |	\
									 ((((uint32)(a)) & 0x000000FFL) << 24) ))
	#define LilWord(a)		(      ( ((((uint16)(a)) & 0x00FF) << 8) |	\
									 ((((uint16)(a)) & 0xFF00) >> 8) ))

	#define BigLong(a)		(a)
	#define	BigWord(a)		(a)

	#define LilLong2Native(a)	((a) = ( ((((uint32)(a)) & 0xFF000000L) >> 24) |	\
										 ((((uint32)(a)) & 0x00FF0000L) >>  8) |	\
										 ((((uint32)(a)) & 0x0000FF00L) <<  8) |	\
										 ((((uint32)(a)) & 0x000000FFL) << 24) ))
	#define LilWord2Native(a)	((a) = ( ((((uint16)(a)) & 0x00FF) << 8) |	\
										 ((((uint16)(a)) & 0xFF00) >> 8) ))

	#define	BigLong2Native(a)	((a)=(a))
	#define	BigWord2Native(a)	((a)=(a))

#endif

#define MakeBigLong(a)	((a) = BigLong((a)))
#define MakeBigWord(a)	((a) = BigWord((a)))
#define MakeLilLong(a)	((a) = LilLong((a)))
#define MakeLilWord(a)	((a) = LilWord((a)))

/************************** ECHIDNA COMPATABLITY **************************/

#if _EL_CPU_iAPx86__
   #ifndef __iAPx86__
      #define __iAPx86__	1
   #endif
#endif

#if _EL_CPU_R3000___
	#ifndef __R3000__
		#define __R3000__
	#endif
#endif

#if _EL_CPU_M68000__
   #ifndef __M68000__
      #define __M68000__ 1
   #endif
#endif

#if _EL_CPU_r4400__
	 #ifndef __echidna_R4400__
		#define __echidna_R4400__ 1
	 #endif
#endif

#if _EL_OS_MSDOS__
   #ifndef __MSDOS__
      #define __MSDOS__	1
   #endif
#endif

#if _EL_OS_PSXOS__
   #ifndef __PSXOS__
      #define __PSXOS__	1
   #endif
#endif

#if _EL_OS_AMIGAOS__
   #ifndef __AMIGAOS__
      #define __AMIGAOS__	1
   #endif
#endif

#if _EL_OS_MACOS__
   #ifndef __MACOS__
      #define __MACOS__	1
   #endif
#endif

#if _EL_OS_IRIX53__
	 #ifndef __echidna_SGI__
	 	#define __echidna_SGI__ 1
	 #endif
#endif

#if __WATCOMC__
#define	__TURBOC__		0
#define	__ZTC__			0
#define	__MACSC__		0
#define	LATTICE			0
#define	__AMIGAOS__		0
#define	__NOT_ANSI__	0
#define DOS386			   0
#define __MACOS__		   0
#define	AZTEC_C			0
#endif

#if (__WATCOMC__ && __DOS__ && __386__ && !defined(__MSDOS32X__))
#define __MSDOS32X__ 1
#endif

#define far32  FAR32
#define huge32  HUGE32

/*******************************************************/


#ifdef __cplusplus
}
#endif

#endif /* EL_PLATFORM_H */

