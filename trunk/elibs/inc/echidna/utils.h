/*************************************************************************
 *                                                                       *
 *                                UTILS.H                                *
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
		Stuff that's generally useful but doesn't deserve it's own
		library


   PROGRAMMERS
		Gregg A. Tavares

   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/11/96 GAT: Created.

 *************************************************************************/

#ifndef EL_UTILS_H
#define EL_UTILS_H
/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#ifndef _EL_PLAT_SONY__
   #include <math.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
/*************************** C O N S T A N T S ***************************/

// common angles for 256-value circle
#define DEGREES_120		0x55
#define DEGREES_60		0x2a
#define DEGREES_30		0x15
#define DEGREES_0		0x00
#define DEGREES_45		0x20
#define DEGREES_90		0x40
#define DEGREES_135		0x60
#define DEGREES_180		0x80
#define DEGREES_225		0xa0
#define DEGREES_270		0xc0
#define DEGREES_315		0xe0
#define DEGREES_360		0x100

// common angles for 4096-value circle
#define DEGREES4096_120		0x555
#define DEGREES4096_60		0x2aa
#define DEGREES4096_30		0x155
#define DEGREES4096_0		0x000
#define DEGREES4096_45		0x200
#define DEGREES4096_90		0x400
#define DEGREES4096_135		0x600
#define DEGREES4096_180		0x800
#define DEGREES4096_225		0xa00
#define DEGREES4096_270		0xc00
#define DEGREES4096_315		0xe00
#define DEGREES4096_360		0x1000

/******************************* T Y P E S *******************************/


/***************************** G L O B A L S *****************************/

// extern float gSinTable64[];
// extern float gSinTable256[];
extern float gSinTable1024[];

/****************************** M A C R O S ******************************/

#define	UTL_ABS(value)	(((value) < 0) ? -(value) : (value))
#define	UTL_MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define	UTL_MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define UTL_SIGN(value)	(((value) > 0) - ((value) < 0))
#define UTL_SWAP(a,b)   {(a) ^= (b); (b) ^= (a); (a) ^= (b);}	// mdw -- c.f. Graphics Gems I pp.436 for storage-free swapping

#define UTL_CHANGEUNITS(v,oldUnits,newUnits)	(((v)*(newUnits))/(oldUnits))
#define UTL_TERPF(a,l,h)  		((l)+(((h)-(l))*(a)))

#if 0
	#define UTL_SIN256_priv64(deg)	(gSinTable64[(deg)])
	#define UTL_SIN256_priv128(deg)	(((deg) <=  64) ? (UTL_SIN256_priv64(deg) ) :   (UTL_SIN256_priv64(128 - (deg))))
	#define UTL_SIN256_priv256(deg)	(((deg) <= 128) ? (UTL_SIN256_priv128(deg)) :  -(UTL_SIN256_priv128((deg) - 128)))
	#define UTL_SIN256_priv(deg)	(((deg) <= 256) ? (UTL_SIN256_priv256(deg)) :   (UTL_SIN256_priv256((deg) % 256)))
	#define UTL_SIN256(deg)			((((int)(deg)) >=   0) ? (UTL_SIN256_priv((int)(deg))) : (-(UTL_SIN256_priv(-(((int)(deg)))))))
	#define UTL_COS256(deg)			UTL_SIN256(((int)(deg)) + 64)
#else
	//
	// these routines take a number from 0-255 and return a float (0-1)
	// They will except any number 1 = 257 = 513 = -511
	//
	#define	UTL_SIN256(deg)			UTL_SIN1024(((int)(deg)) * 4)
	#define	UTL_COS256(deg)			UTL_SIN256((deg) + 64)
#endif

#if 0
	#define UTL_SIN1024_priv64(deg)		(gSinTable256[(deg)])
	#define UTL_SIN1024_priv128(deg)	(((deg) <= 256) ? (UTL_SIN1024_priv64(deg) ) :   (UTL_SIN1024_priv64(512 - (deg))))
	#define UTL_SIN1024_priv256(deg)	(((deg) <= 512) ? (UTL_SIN1024_priv128(deg)) :  -(UTL_SIN1024_priv128((deg) - 512)))
	#define UTL_SIN1024_priv(deg)		(((deg) <= 1024) ? (UTL_SIN1024_priv256(deg)) :   (UTL_SIN1024_priv256((deg) % 1024)))
		//
		// these routines take a number from 0-1023 and return a float (0-1)
		// They will except any number so 1 = 1025 = 2049 = -1023
		//
	#define UTL_SIN1024(deg)			((((int)(deg)) >=   0) ? (UTL_SIN1024_priv((int)(deg))) : (-(UTL_SIN1024_priv(-(((int)(deg)))))))
	#define UTL_COS1024(deg)			UTL_SIN1024(((int)(deg)) + 256)
#else
		//
		// these routines take a number from 0-1023 and return a float (0-1)
		// They will except any number so 1 = 1025 = 2049 = -1023
		//
	#define UTL_SIN1024(deg)			gSinTable1024[(((int)(deg)) & 0x3FF)]
	#define UTL_COS1024(deg)			UTL_SIN1024(((int)(deg)) + 256)
#endif

	//
	// these routines take a number from 0-360 and return a float (0-1)
	// They will except any number so 1 = 361 = 721 = -359
	//
#define	UTL_COSF(deg)			UTL_SINF((deg)+90)

#if _7_OS_3DO_M2__
	#define	UTL_tanf	tanf
	#define	UTL_fabsf	fabsf
#else
	#define	UTL_tanf(a)		((float)tan((float)(a)))
	#define	UTL_fabsf(a)	((float)fabs((float)(a)))
#endif

#if _EL_PLAT_SONY__
	 #define UTL_isspace(c)	(' '==(c) || '\t' == (c) || '\n' == (c))
	 #define UTL_isdigit(c)	('0' <= (c) && (c) <= '9')
#else
	 #define UTL_isspace(c)	isspace(c)
	 #define UTL_isdigit(c)	isdigit(c)
#endif

/************************** P R O T O T Y P E S **************************/

extern float UTL_floor (float v);
extern float UTL_ceil (float v);
extern float UTL_fmod (float v, float d);
extern float UTL_SINF (float degrees);
extern int16 UTL_PseudoRandom (void);
extern int16 UTL_ReallyRandom (void);
extern void UTL_ResetPseudoRandomNumber (void);
extern uint8 UTL_FindAngle (int32 x, int32 y);
extern int32 UTL_SIN (uint8 angle);
extern int32 UTL_COS (uint8 angle);
int32 UTL_Sin4096 (int32 angle);
int32 UTL_Cos4096 (int32 angle);
int32 UTL_FindAngle4096 (int32 dx, int32 dy);
int8 UTL_AngleToDirection4096(int32 angle);

#ifdef __cplusplus
}
#endif
#endif /* EL_UTILS_H */





