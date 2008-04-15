/*************************************************************************
 *                                                                       *
 *                               maclang.h                               *
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


   PROGRAMMERS
		Gregg A. Tavares

   FUNCTIONS

   TABS : 5 9

   HISTORY
		01/15/01 GAT: Created
*/ 
#ifndef ECHIDNA_MLANG_H
#define ECHIDNA_MLANG_H
/**************************** i n c l u d e s ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

/*************************** c o n s t a n t s ***************************/


/******************************* t y p e s *******************************/


/***************************** g l o b a l s *****************************/


/****************************** m a c r o s ******************************/


/************************** p r o t o t y p e s **************************/

#ifdef __cplusplus

extern "C" {
#endif


void MLANG_AddVariable (const char* name, const char* value);
void MLANG_RemoveVariable (const char* name);
char* MLANG_SubVariables (const char* str, const char* inputPath);

#ifdef __cplusplus
}

#include <string>
using std::string;

bool MLANG_GetVariable (const string& var, string& value);
bool MLANG_SubVariables (string& str, const char* inputPath);


#endif

#endif /* ECHIDNA_MLANG_H */




