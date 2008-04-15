/*************************************************************************
 *                                                                       *
 *                              MEMFILE.CPP                              *
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


   FUNCTIONS

   TABS : 5 9

   HISTORY
        03/28/96 : Created.

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include "echidna/memfile.h"
#include "echidna/eio.h"

/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/


/************************** P R O T O T Y P E S **************************/


/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/


/**************************** R O U T I N E S ****************************/

// for creating your own mem file
void MEMFILE_Init(MEMFILE* mf, void* buf, long len)
{
    memset (mf, 0, sizeof (MEMFILE));

    mf->buffer    = (uint8*)buf;
    mf->curPtr    = mf->buffer;
    mf->size      = len;
    mf->bytesLeft = len;
}

MEMFILE *MEMFILE_Load (const char *filename)
{
    MEMFILE* mf = NULL;
    int     fh;
    long    len;

    fh = EIO_ReadOpen(filename);
    if (fh != (-1))
    {
        len = EIO_FileLength(fh);
        if (len)
        {
            mf = (MEMFILE*)malloc (len + sizeof (MEMFILE));
            if (mf)
            {
                MEMFILE_Init(mf, ((uint8*)mf)+sizeof (MEMFILE), len);

                EIO_Read (fh, mf->buffer, len);
            }
        }
        EIO_Close (fh);
    }
    return mf;
}

void MEMFILE_Close (MEMFILE *mf)
{
    free (mf);
}

int MEMFILE_Read(MEMFILE *mf, void *buf, long len)
{
    if (mf->bytesLeft)
    {
        long    copyBytes;

        copyBytes = (mf->bytesLeft < len) ? mf->bytesLeft : len;

        memcpy (buf, mf->curPtr, copyBytes);

        mf->bytesLeft -= copyBytes;
        mf->curPtr    += copyBytes;

        return copyBytes;
    }
    return 0;
}

int MEMFILE_Seek (MEMFILE *mf, long pos, int type)
{
    switch (type)
    {
    case SEEK_SET:
        mf->curPtr    = mf->buffer + pos;
        mf->bytesLeft = mf->size   - pos;
        break;
    case SEEK_CUR:
        mf->curPtr    += pos;
        mf->bytesLeft -= pos;
        break;
    case SEEK_END:
        mf->curPtr     = mf->buffer + mf->size + pos;
        mf->bytesLeft  = -pos;
        break;
    }
    return mf->size - mf->bytesLeft;
}


