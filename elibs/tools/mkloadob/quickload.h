/*=======================================================================*
 |   file name : quickload.h
 |-----------------------------------------------------------------------*
 |   function  : load a file into memory
 |-----------------------------------------------------------------------*
 |   author    : Gregg Tavares
 |-----------------------------------------------------------------------*

	The Echidna Copyright
	
	Copyright 1991-2003 Echidna, Inc. All rights reserved.
	
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are
	met:
	
	* Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.
	
	* Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.
	
	THIS SOFTWARE IS PROVIDED BY Echidna ``AS IS'' AND ANY EXPRESS OR IMPLIED
	WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
	NO EVENT SHALL Echidna OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
	NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
	THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
	
	The views and conclusions contained in the software and documentation are
	those of the authors and should not be interpreted as representing
	official policies, either expressed or implied, of Echidna or
	Echidna, Inc.

 *=======================================================================*/

#ifndef QUICKLOAD_H
#define QUICKLOAD_H

/**************************** i n c l u d e s ****************************/


/*************************** c o n s t a n t s ***************************/


/******************************* t y p e s *******************************/


/***************************** g l o b a l s *****************************/


/****************************** m a c r o s ******************************/


/************************** p r o t o t y p e s **************************/

void* QuickLoadFile (const char* filename, unsigned int alignment = 1);

#endif /* QUICKLOAD_H */




