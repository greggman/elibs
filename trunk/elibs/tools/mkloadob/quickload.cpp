/*=======================================================================*
 |   file name : quickload.cpp
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

/**************************** i n c l u d e s ****************************/

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "quickload.h"

/*************************** c o n s t a n t s ***************************/


/******************************* t y p e s *******************************/


/************************** p r o t o t y p e s **************************/


/***************************** g l o b a l s *****************************/


/****************************** m a c r o s ******************************/


/**************************** r o u t i n e s ****************************/


#define QLOAD_AlignData(data)   \
        {   \
            unsigned int offAlignment = ((unsigned int)(data)) % alignment;  \
            if (offAlignment)   \
            {   \
                (data) = (void*)(((unsigned int)(data)) + (alignment - offAlignment));   \
            }   \
        }


/*************************************************************************
                              QuickLoadFile
 *************************************************************************

   SYNOPSIS
		void* QuickLoadFile (char* filename, unsigned int alignement = 1)

   PURPOSE
  		Load a file during development into memory simply.

   INPUT
		filename : file to load
        alignment: memory alignment (eg, 4 = 4 byte boundry)

   OUTPUT
		None

   EFFECTS
		None

   RETURNS


   SEE ALSO


   HISTORY
		05/17/02 GAT: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void* QuickLoadFile (const char* filename, unsigned int alignment)
{
    if (!alignment) alignment = 1;

	int fh;
	long size;
	void* data = NULL;;
		
	fprintf (stderr, "loading %s\n", filename);
	
	fh = open (filename, O_RDONLY);
	if (fh < 0)
	{
		fprintf (stderr, "could not open file %s\n", filename);
		goto error;
	}
	
	size = lseek (fh, 0L, SEEK_END);
	if (size <= 0 || lseek (fh, 0, SEEK_SET) < 0)
	{
		fprintf (stderr, "seek failed for file %s\n", size, filename);
		goto error;
	}
		
	data = malloc ((size_t)size + alignment);
	if (!data)
	{
		fprintf (stderr, "could not allocate %d bytes for file %s\n", size, filename);
		goto error;
	}
	
	QLOAD_AlignData (data);
	
	if (read (fh, data, (size_t)size) != size)
	{
		free (data);
		fprintf (stderr, "could not read %d bytes for file %s\n", size, filename);
	}
	close (fh);

error:	
	return data;
}


